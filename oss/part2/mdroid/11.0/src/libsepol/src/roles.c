#include <stdlib.h>
#include <string.h>

#include <sepol/policydb/hashtab.h>
#include <sepol/policydb/policydb.h>

#include <sepol/debug.h>  //STDLIBC_INTEGER
#include <sepol/handle.h>  //STDLIBC_INTEGER

/* Check if a role exists */
int sepol_role_exists(sepol_handle_t * handle __attribute__ ((unused)),
		      sepol_policydb_t * p, const char *role, int *response)
{

	policydb_t *policydb = &p->p;
	*response = (hashtab_search(policydb->p_roles.table, role) != NULL);

	return STATUS_SUCCESS;
}

/* Fill an array with all valid roles */
int sepol_role_list(sepol_handle_t * handle,
		    sepol_policydb_t * p, char ***roles, unsigned int *nroles)
{

	policydb_t *policydb = &p->p;
	unsigned int tmp_nroles = policydb->p_roles.nprim;
	char **tmp_roles = (char **)malloc(tmp_nroles * sizeof(char *));
	char **ptr;
	unsigned int i;
	if (!tmp_roles)
		goto omem;

	for (i = 0; i < tmp_nroles; i++) {
		tmp_roles[i] = strdup(policydb->p_role_val_to_name[i]);
		if (!tmp_roles[i])
			goto omem;
	}

	*nroles = tmp_nroles;
	*roles = tmp_roles;

	return STATUS_SUCCESS;

      omem:
	ERR(handle, "out of memory, could not list roles");

	ptr = tmp_roles;
	while (ptr && *ptr)
		free(*ptr++);
	free(tmp_roles);
	return STATUS_ERR;
}
