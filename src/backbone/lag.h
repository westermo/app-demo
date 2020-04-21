#ifndef _LAG_H
#define _LAG_H

#include <ev.h>
#include <team.h>

#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <net/if.h>
#include <sys/socket.h>

struct backbone_id {
	uint8_t station[ETH_ALEN];
	uint8_t head[ETH_ALEN];
	uint8_t id;
} __attribute__ (( packed ));

static inline int backbone_id_cmp(const struct backbone_id *a,
				  const struct backbone_id *b)
{
	static const uint8_t zeroaddr[ETH_ALEN] = { 0 };
	bool ahead, bhead;

	ahead = !!memcmp(a->head, zeroaddr, ETH_ALEN);
	bhead = !!memcmp(b->head, zeroaddr, ETH_ALEN);

	/* Any attached ID is better than any detached ID. */
	if (ahead ^ bhead)
		return ahead ? 1 : -1;

	/* If both are attached, lower head MAC is better. */
	if (ahead && bhead)
		return -memcmp(a->head, b->head, ETH_ALEN);

	/* Two unattached stations are equal. */
	return 0;
}

struct backbone_hello {
	uint8_t  slow_subtype;

	uint8_t  oui[3];
	uint8_t  protocol;

	struct backbone_id id;
} __attribute__ (( packed ));

struct lag_port {
	struct lag *lag;
	struct team_port *tp;
	unsigned int ifindex;
	bool enabled;

	struct ev_io io;
	int fd;

	struct backbone_id neigh;
	struct timespec tstamp;
};

bool lag_port_has_link  (struct lag_port *p);
bool lag_port_is_enabled(struct lag_port *p);
bool lag_port_enable    (struct lag_port *p, bool enable);

struct lag_config;

struct lag {
	struct team_handle *th;
	unsigned int ifindex;
	struct lag_port *port[2];
	struct lag_config *conf;

	struct ev_io team_io;

	struct {
		struct ev_timer timer;

		struct mmsghdr        mmsg[2];
		struct sockaddr_ll    addr[2];
		struct iovec          iovec;
		struct backbone_hello pkt;
	} tx;
};

struct lag_config {
	char    name[IFNAMSIZ];
	uint8_t hwaddr[ETH_ALEN];

	const char *port[2];

	struct ev_loop *loop;
	int  (*backbone_id)(struct backbone_id *id);
	bool (*update)(void);
};

int  lag_new (struct lag_config *conf, struct lag **lagp);
void lag_free(struct lag *lag);

#endif	/* _LAG_H */
