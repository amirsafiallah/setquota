Set Quota C Source Code For Linux
=================================

Usage
-----

Change the following variables, compile then run as `root`.

    const struct ldap_quota lq = {.fs = "/",
                .quotaBhardlimit=100,
                .quotaBsoftlimit=100,
                .quotaIhardlimit=100,
                .quotaIsoftlimit = 100};
    uid_t uid = 1001;

- `fs` is the device file or mountpoint the policy applies to.
- `quotaBhardlimit`, `quotaBsoftlimit`, `quotaIhardlimit` and `quotaIsoftlimit` are as defined by
  `quotactl(2)`:
  - `B` expresses a number of blocks (size limit), whereas
	`I` is a limit on the number of inodes.
  - `softlimit` is a threshold after which the user gets warnings,
	whereas `hard` limits cannot be exceeded.
- `uid` uid of user which policy is applied to.
