/*
 *  Copyright (C) 2007-2010 Lawrence Livermore National Security, LLC.
 *  Copyright (C) 2007 The Regents of the University of California.
 *  Produced at Lawrence Livermore National Laboratory (cf, DISCLAIMER).
 *  Written by Brian Behlendorf <behlendorf1@llnl.gov>.
 *  UCRL-CODE-235197
 *
 *  This file is part of the SPL, Solaris Porting Layer.
 *
 *  The SPL is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License as published by the
 *  Free Software Foundation; either version 2 of the License, or (at your
 *  option) any later version.
 *
 *  The SPL is distributed in the hope that it will be useful, but WITHOUT
 *  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 *  for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with the SPL.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  Solaris Porting Layer (SPL) Credential Implementation.
 */

#include <sys/cred.h>

static int
cr_groups_search(const struct group_info *group_info, kgid_t grp)
{
	unsigned int left, right, mid;
	int cmp;

	if (!group_info)
		return (0);

	left = 0;
	right = group_info->ngroups;
	while (left < right) {
		mid = (left + right) / 2;
		cmp = KGID_TO_SGID(grp) -
		    KGID_TO_SGID(GROUP_AT(group_info, mid));

		if (cmp > 0)
			left = mid + 1;
		else if (cmp < 0)
			right = mid;
		else
			return (1);
	}
	return (0);
}

/* Hold a reference on the credential */
void
crhold(cred_t *cr)
{
	(void) get_cred((const cred_t *)cr);
}

/* Free a reference on the credential */
void
crfree(cred_t *cr)
{
	put_cred((const cred_t *)cr);
}

/* Return the number of supplemental groups */
int
crgetngroups(const cred_t *cr)
{
	struct group_info *gi;
	int rc;

	gi = cr->group_info;
	rc = gi->ngroups;
#ifndef HAVE_GROUP_INFO_GID
	/*
	 * For Linux <= 4.8,
	 * crgetgroups will only returns gi->blocks[0], which contains only
	 * the first NGROUPS_PER_BLOCK groups.
	 */
	if (rc > NGROUPS_PER_BLOCK) {
		WARN_ON_ONCE(1);
		rc = NGROUPS_PER_BLOCK;
	}
#endif
	return (rc);
}

/*
 * Return an array of supplemental gids.  The returned address is safe
 * to use as long as the caller has taken a reference with crhold().
 *
 * Linux 4.9 API change, group_info changed from 2d array via ->blocks to 1d
 * array via ->gid.
 */
gid_t *
crgetgroups(const cred_t *cr)
{
	struct group_info *gi;
	gid_t *gids = NULL;

	gi = cr->group_info;
#ifdef HAVE_GROUP_INFO_GID
	gids = KGIDP_TO_SGIDP(gi->gid);
#else
	if (gi->nblocks > 0)
		gids = KGIDP_TO_SGIDP(gi->blocks[0]);
#endif
	return (gids);
}

/* Check if the passed gid is available in supplied credential. */
int
groupmember(gid_t gid, const cred_t *cr)
{
	struct group_info *gi;
	int rc;

	gi = cr->group_info;
	rc = cr_groups_search(gi, SGID_TO_KGID(gid));

	return (rc);
}

/* Return the effective user id */
uid_t
crgetuid(const cred_t *cr)
{
	return (KUID_TO_SUID(cr->fsuid));
}

/* Return the real user id */
uid_t
crgetruid(const cred_t *cr)
{
	return (KUID_TO_SUID(cr->uid));
}

/* Return the effective group id */
gid_t
crgetgid(const cred_t *cr)
{
	return (KGID_TO_SGID(cr->fsgid));
}

/* Return the initial user ns or nop_mnt_idmap */
zidmap_t *
zfs_get_init_idmap(void)
{
#ifdef HAVE_IOPS_CREATE_IDMAP
	return ((zidmap_t *)&nop_mnt_idmap);
#else
	return ((zidmap_t *)&init_user_ns);
#endif
}

EXPORT_SYMBOL(zfs_get_init_idmap);
EXPORT_SYMBOL(crhold);
EXPORT_SYMBOL(crfree);
EXPORT_SYMBOL(crgetuid);
EXPORT_SYMBOL(crgetruid);
EXPORT_SYMBOL(crgetgid);
EXPORT_SYMBOL(crgetngroups);
EXPORT_SYMBOL(crgetgroups);
EXPORT_SYMBOL(groupmember);
