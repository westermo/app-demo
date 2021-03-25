/* On-wire format:
 *
 * Ethernet:
 *   .-------.-------.---------.
 *   | DA(6) | SA(6) | Type(2) |
 *   '-------'-------'---------'
 *   DA:      01:80:c2:00:00:02, "Slow protocols".
 *   SA:      MAC of sending interface.
 *   Type:    0x8809, "Slow protocols".
 *
 * Slow Protocol:
 *   .------------.--------.-------------.
 *   | Subtype(1) | OUI(3) | Protocol(1) |
 *   '------------'--------'-------------'
 *   Subtype:  10, "Organization Specific Slow Protocol" (OSSP).
 *   OUI:      00:07:7c, "Westermo".
 *   Protocol: 1, "Backbone".
 *
 * usbboot:
 *   .------------.---------.-------.
 *   | Station(6) | Head(6) | ID(1) |
 *   '------------'---------'-------'
 *   Station: MAC of sender.
 *   Head:    MAC of head that sender is attached to, or zero if detached.
 *   ID:      ID on backbone, i.e. distance from head when attached.
 */

#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <linux/if_packet.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <netinet/ether.h>
#include <sys/queue.h>
#include <sys/socket.h>

#include "lag.h"
#include "util.h"

int lag_tx(struct lag *lag);
int lag_carrier_update(struct lag *lag);

static struct sock_filter usbboot_filter[] = {
#define ASM_BPF_JMP(_op, _t, _f, _k) { BPF_JMP | (_op), (_t), (_f), (_k)   }
#define ASM_BPF_LD(_w, _m, _offs)    { BPF_LD | (_w) | (_m), 0, 0, (_offs) }

	/* Packet must be at least 18 bytes, a slow protocol header
	 * (5) and a usbboot message (13) */
	ASM_BPF_LD  (BPF_W, BPF_LEN, 0),
	ASM_BPF_JMP (BPF_JGE, 0, 5, 18),

	/* Slow protocol subtype must be OSSP. */
	ASM_BPF_LD  (BPF_B, BPF_ABS, 0),
	ASM_BPF_JMP (BPF_JEQ, 0, 3, 10),

	/* OUI Must be Westermo, protocol must be usbboot. */
	ASM_BPF_LD  (BPF_W, BPF_ABS, 1),
	ASM_BPF_JMP (BPF_JEQ, 0, 1, 0x00077c01),

	/* Truncate to 18 bytes. */
	{ BPF_RET, 0, 0, 18 },

	/* Filter */
	{ BPF_RET, 0, 0,  0 },
};


bool lag_port_has_link(struct lag_port *p)
{
	return team_is_port_link_up(p->tp);
}

bool lag_port_is_enabled(struct lag_port *p)
{
	bool enabled;

	team_get_port_enabled(p->lag->th, p->ifindex, &enabled);
	return enabled;
}

bool lag_port_enable(struct lag_port *p, bool enable)
{
	int err;

	if (p->enabled == enable)
		return false;

	err = team_set_port_enabled(p->lag->th, p->ifindex, enable);
	if (err) {
		ERR("Unable to %sable port, exiting", enable ? "en" : "dis");
		ev_break(p->lag->conf->loop, EVBREAK_ALL);
		return false;
	}

	p->enabled = enable;
	lag_carrier_update(p->lag);
	return true;
}


static void lag_port_ev_rx(struct ev_loop *loop, ev_io *w, int revents)
{
	struct lag_port *p = container_of(w, struct lag_port, io);
	struct usbboot_hello pkt;
	ssize_t len;

	len = read(p->fd, &pkt, sizeof(pkt));
	if (len < 0) {
		ERR("%s: Unable to read incoming packet: %zd",
		    p->lag->conf->name, len);
		return;
	}

	clock_gettime(CLOCK_MONOTONIC, &p->tstamp);

	/* Has our neighbor changed? */
	if (!memcmp(&pkt.id, &p->neigh, sizeof(pkt.id)))
		return;

	/* Yes, store the new info... */
	memcpy(&p->neigh, &pkt.id, sizeof(p->neigh));

	/* ...and tell our parent. Our ID could have changed. If so,
	 * tell our neighbors about it. */
	if (p->lag->conf->update())
		lag_tx(p->lag);
}

static int lag_port_setup_io(struct lag_port *p)
{
	struct sockaddr_ll addr = { 0 };
	struct sock_fprog fprog;

	p->fd = socket(AF_PACKET, SOCK_DGRAM, htons(ETH_P_SLOW));
	if (p->fd < 0)
		return 1;

	if (fcntl(p->fd, F_SETFL, fcntl(p->fd, F_GETFL, 0) | O_NONBLOCK)) {
		close(p->fd);
		return 1;
	}

	fprog.filter = usbboot_filter;
	fprog.len = sizeof(usbboot_filter) / sizeof(usbboot_filter[0]);
	if (setsockopt(p->fd, SOL_SOCKET, SO_ATTACH_FILTER,
		       &fprog, sizeof(fprog))) {
		close(p->fd);
		return 1;
	}

	addr.sll_family   = AF_PACKET;
	addr.sll_protocol = htons(ETH_P_SLOW);
	addr.sll_ifindex  = p->ifindex;
	if (bind(p->fd, (struct sockaddr *)&addr, sizeof(addr))) {
		close(p->fd);
		return 1;
	}

	ev_io_init(&p->io, lag_port_ev_rx, p->fd, EV_READ);
	ev_io_start(p->lag->conf->loop, &p->io);
	return 0;
}

static int lag_port_new(struct lag *lag, const char *name,
			 struct lag_port **pp)
{
	struct lag_port *p;
	unsigned int ifindex;
	int err;

	if (!(ifindex = if_nametoindex(name))) {
		err = ENODEV;
		goto err;
	}

	p = calloc(1, sizeof(*p));
	if (!p) {
		err = ENOMEM;
		goto err;
	}

	p->lag = lag;
	p->ifindex = ifindex;

	err = team_port_add(lag->th, p->ifindex);
	if (err)
		goto err_free;

	team_set_port_enabled(lag->th, p->ifindex, false);

	*pp = p;
	return 0;

err_free:
	free(p);
err:
	return err;
}

static void lag_port_free(struct lag_port *p)
{
	ev_io_stop(p->lag->conf->loop, &p->io);

	team_port_remove(p->lag->th, p->ifindex);
	free(p);
}

static void lag_ev_tx_timer(struct ev_loop *loop, ev_timer *timer, int revents)
{
	struct lag *lag = container_of(timer, struct lag, tx.timer);
	struct timespec now;
	int i;

	clock_gettime(CLOCK_MONOTONIC, &now);

	for (i = 0; i < 2; i++) {
		if (!lag->port[i]->tstamp.tv_sec)
			continue;

		if (now.tv_sec - lag->port[i]->tstamp.tv_sec < 2)
			continue;

		/* Neighbor lost. */
		memset(&lag->port[i]->tstamp, 0, sizeof(lag->port[i]->tstamp));
		memset(&lag->port[i]->neigh, 0, sizeof(lag->port[i]->neigh));
		lag->conf->update();
	}

	if (lag->conf->usbboot_id(&lag->tx.pkt.id)) {
		ERR("ERR: ID not available, can't send hello");
		return;
	}

	lag_tx(lag);
}

static int lag_team_change(struct team_handle *th, void *func_priv,
			   team_change_type_mask_t type_mask)
{
	/* struct lag *lag = func_priv; */

	return 0;
}

static const struct team_change_handler lag_team_handler = {
	.func = lag_team_change,
	.type_mask = TEAM_ANY_CHANGE,
};

static void lag_ev_team(struct ev_loop *loop, ev_io *w, int revents)
{
	struct lag *lag = container_of(w, struct lag, team_io);
	int err;

	err = team_handle_events(lag->th);
	if (err)
		ERR("Unable to handle team events: %d\n", err);
}


static int lag_sync_hwaddr(struct lag *lag)
{
	static const uint8_t zeroaddr[ETH_ALEN] = { 0 };

	if (!memcmp(lag->conf->hwaddr, zeroaddr, ETH_ALEN))
		return team_hwaddr_get(lag->th, lag->ifindex,
				       (char *)lag->conf->hwaddr, ETH_ALEN);
	else
		return team_hwaddr_set(lag->th, lag->ifindex,
				       (char *)lag->conf->hwaddr, ETH_ALEN);
}

int lag_carrier_update(struct lag *lag)
{
	/* Signal to upper layers whether there is any chance of
	 * forwarding packets over this link or not. */
	return team_carrier_set(lag->th,
				lag->port[0]->enabled || lag->port[1]->enabled);
}

int lag_tx(struct lag *lag)
{
	int ret;

	/* Everything is prepared at this point, the mmsghdr is setup
	 * to send out one message through each LAG port. */
	ret = sendmmsg(lag->port[0]->fd, lag->tx.mmsg, 2, 0);
	if (ret < 2) {
		WARN("%s: Unable to send hello: %d", lag->conf->name, ret);
		return EIO;
	}

	return 0;
}

static void lag_setup_tx(struct lag *lag)
{
	static const struct sockaddr_ll addr = {
		.sll_family = AF_PACKET,
		.sll_halen = ETH_ALEN,
		.sll_addr = { 0x01, 0x80, 0xc2, 0x00, 0x00, 0x02 },
	};
	struct usbboot_hello *hello = &lag->tx.pkt;
	int i;

	/* Prepare hello packet header. */
	hello->slow_subtype = 10;
	hello->oui[0]       = 0x00;
	hello->oui[1]       = 0x07;
	hello->oui[2]       = 0x7c;
	hello->protocol     = 0x01;

	/* Point the io vector to the packet buffer. */
	lag->tx.iovec.iov_base = &lag->tx.pkt;
	lag->tx.iovec.iov_len = sizeof(lag->tx.pkt);

	for (i = 0; i < 2; i++) {
		/* Fill in outgoing interface */
		lag->tx.addr[i] = addr;
		lag->tx.addr[i].sll_protocol = htons(ETH_P_SLOW);
		lag->tx.addr[i].sll_ifindex = lag->port[i]->ifindex;

		lag->tx.mmsg[i].msg_hdr.msg_namelen = sizeof(addr);
		lag->tx.mmsg[i].msg_hdr.msg_iov = &lag->tx.iovec;
		lag->tx.mmsg[i].msg_hdr.msg_iovlen = 1;
		lag->tx.mmsg[i].msg_hdr.msg_name = &lag->tx.addr[i];
	}
}

static int lag_setup_io(struct lag *lag)
{
	int err, fd;

	lag_setup_tx(lag);

	err =         lag_port_setup_io(lag->port[0]);
	err = err ? : lag_port_setup_io(lag->port[1]);
	if (err)
		return err;

	err = team_change_handler_register(lag->th, &lag_team_handler, lag);
	if (err)
		return err;

	fd = team_get_event_fd(lag->th);
	if (fd < 0)
		return fd;

	ev_io_init(&lag->team_io, lag_ev_team, fd, EV_READ);
	ev_io_start(lag->conf->loop, &lag->team_io);

	ev_timer_init(&lag->tx.timer, lag_ev_tx_timer, 1., 1.);
	ev_timer_start(lag->conf->loop, &lag->tx.timer);
	return 0;
}

int lag_new(struct lag_config *conf, struct lag **lagp)
{
	struct team_port *tp;
	struct lag *lag;
	int err = ENOMEM;

	lag = calloc(1, sizeof(*lag));
	if (!lag)
		goto err;

	lag->conf = conf;

	lag->th = team_alloc();
	if (!lag->th)
		goto err_free;

	err = team_create(lag->th, conf->name);
	if (err)
		goto err_team_free;

	lag->ifindex = if_nametoindex(conf->name);

	err = lag_sync_hwaddr(lag);
	if (err)
		goto err_team_destroy;

	err = team_init(lag->th, lag->ifindex);
	if (err)
		goto err_team_destroy;

	err = team_set_mode_name(lag->th, "loadbalance");
	if (err)
		goto err_team_destroy;

	err = lag_port_new(lag, conf->port[0], &lag->port[0]);
	if (err)
		goto err_team_destroy;

	err = lag_port_new(lag, conf->port[1], &lag->port[1]);
	if (err)
		goto err_team_destroy;

	err = lag_carrier_update(lag);
	if (err)
		goto err_team_destroy;

	err = team_refresh(lag->th);
	if (err)
		goto err_team_destroy;

	team_for_each_port(tp, lag->th) {
		unsigned int ifindex = team_get_port_ifindex(tp);

		if (ifindex == lag->port[0]->ifindex)
			lag->port[0]->tp = tp;
		else if (ifindex == lag->port[1]->ifindex)
			lag->port[1]->tp = tp;
	}

	err = lag_setup_io(lag);
	if (err)
		goto err_team_destroy;

	*lagp = lag;
	return 0;

err_team_destroy:
	if (lag->port[1])
		lag_port_free(lag->port[0]);
	if (lag->port[0])
		lag_port_free(lag->port[0]);

	team_destroy(lag->th);
err_team_free:
	team_free(lag->th);
err_free:
	free(lag);
err:
	return err;
}

void lag_free(struct lag *lag)
{
	ev_timer_stop(lag->conf->loop, &lag->tx.timer);
	ev_io_stop(lag->conf->loop, &lag->team_io);

	lag_port_free(lag->port[1]);
	lag_port_free(lag->port[0]);

	team_destroy(lag->th);
	team_free(lag->th);
	free(lag);
}
