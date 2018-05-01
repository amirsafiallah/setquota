#include <stdio.h>
#include <linux/quota.h>
#include <sys/quota.h>
#include <syslog.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <mntent.h>
#include <pwd.h>

struct ldap_quota {
    char fs[BUFSIZ]; //File System
    u_int64_t quotaBhardlimit; //Blocks Hard Limit
    u_int64_t quotaBsoftlimit; //Blocks Soft Limit
    u_int64_t quotaIhardlimit; //INodes Hard Limit
    u_int64_t quotaIsoftlimit; //INodes Soft Limit
};

/*return 1 if anything changed*/
void setquota(struct if_dqblk *pDqblk, const struct ldap_quota *pQuota) {
    pDqblk->dqb_bhardlimit = pQuota->quotaBhardlimit;
    pDqblk->dqb_bsoftlimit = pQuota->quotaBsoftlimit;
    pDqblk->dqb_ihardlimit = pQuota->quotaIhardlimit;
    pDqblk->dqb_isoftlimit = pQuota->quotaIsoftlimit;

    pDqblk->dqb_valid |= QIF_BLIMITS;
    pDqblk->dqb_valid |= QIF_ILIMITS;
}

int main() {
    const struct ldap_quota lq = {.fs = "/",
            .quotaBhardlimit=100,
            .quotaBsoftlimit=100,
            .quotaIhardlimit=100,
            .quotaIsoftlimit = 100};
    uid_t uid = 1001;


    char mntdevice[BUFSIZ], mntpoint[BUFSIZ];
    const struct mntent *mnt;
    *mntpoint = *mntdevice = '\0';
    size_t match_size = 0;
    const struct passwd *pwd;
    FILE *fd;

    pwd = getpwuid(uid);

    if ((fd = setmntent("/etc/mtab", "r")) == NULL) {
        perror("Unable to open /etc/mtab");
        return EXIT_FAILURE;
    }

    while ((mnt = getmntent(fd)) != NULL) {
        if (lq.fs == NULL) {
            size_t mnt_len = strlen(mnt->mnt_dir);
            /* If fs is not specified use filesystem with homedir as default
             * Checking the mnt_len-th character in pwd->pw_dir is safe because of the
             * strncmp(2) check before (whose success implies strlen(pwd->pw_dir) >=
             * mntlen)
             */
            if ((strncmp(pwd->pw_dir, mnt->mnt_dir, mnt_len) == 0) &&
                (mnt_len > match_size) &&
                (pwd->pw_dir[mnt_len] == '\0' || pwd->pw_dir[mnt_len] == '/')) {
                strncpy(mntpoint, mnt->mnt_dir, sizeof(mntpoint));
                strncpy(mntdevice, mnt->mnt_fsname, sizeof(mntdevice));
                match_size = mnt_len;
            }
        } else if ((strcmp(lq.fs, mnt->mnt_dir) == 0) ||
                   (strcmp(lq.fs, mnt->mnt_fsname) == 0)) {
            strncpy(mntdevice, mnt->mnt_fsname, sizeof(mntdevice));
        }
    }
    /*The endmntent() function closes the file system description file fd*/
    endmntent(fd);

    struct if_dqblk ndqblk;
    /* Get limits */
    if (quotactl(QCMD(Q_GETQUOTA, USRQUOTA), mntdevice, pwd->pw_uid,
                 (void *) &ndqblk) == -1) {
        perror(strerror(errno));
        return EXIT_FAILURE;
    }

    setquota(&ndqblk, &lq);

    if (quotactl(QCMD(Q_SETQUOTA, USRQUOTA), mntdevice, pwd->pw_uid,
                 (void *) &ndqblk) == -1) {
        perror(strerror(errno));
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}