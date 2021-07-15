Hacking the app-demo Project
============================

This file is a Checklist and things to consider when hacking on this
project.  Maintainers, and project leads, check section Releases.


Contributing
------------

We prefer GitHub pull requests since it allows for auditing all changes
that go into the tree:

  0. If you already have write access to the repo, continue to 2.
  1. Clone the project to your own user on GitHub
  2. Create a branch for your feature/fix, use a descriptive branch name
  3. Prepare your GitHub Pull Request

General guidelines:

  1. Keep commits small and logical:
     - separate white space changes from actual changes
	 - changes to different demos should be different pull requests,
	   unless the pull request is common to more than one
  2. For C code we use Linux kernel coding style, no exceptions
  3. For C code, try building (and running) locally first.  Always set
     strict warning flags to GCC, e.g. `-W -Wall -Wextra`
  4. For shell scripts, run them locally first
  5. For shell scripts, run `shellcheck` to lint
  6. For changes to either C or shell scripts; follow the coding style
     used in the existing code.  Shell script are either tab or four
	 space indent -- never mixed in the same script


Releases
--------

GitHub Releases is how we get `.img` files to test and refer customers
to.  They are completely automated; release-generation, build, as well
as uploading of release artifacts (`.img` files) is triggered by the
`release.yml` GitHub action when a tag is pushed.

Tags must follow the following syntax:

    MAJOR.MINOR.PATCH[-(alpha|beta|rc)NUM]

For details on the versioning scheme, see section Versioning, below.

**Examples:**

  - 1.0.0
  - 1.1.0
  - 1.2.0-alpha1
  - 1.2.0-beta1
  - 1.2.0-rc1

Please note, `-alphaN`, `-betaN`, and `-rcN` are tagged as `prerelease`
in the GitHub Release page, and should only be used for internal testing
at Westermo.


Versioning
----------

General recommendation; the `PATCH` release number should be used for
fixes to existing demos, e.g. a bug/doc fix to the Backbone app.
Whereas a new demo warrants a step in the `MINOR` number.  Stepping the
`MAJOR` number is done only when the underlying base platform is
updated.


### 1.x.y

The 1.x.y series are based on NetBox 2020.02-r0.  The first 1.0.0
release only had one demo, while 1.1.0 had two more.

### 2.x.y

This is planned to be using NetBox 2021.02-r0.


