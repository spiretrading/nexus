import AccountRole from './account-role';

/** Account roles util class */
class AccountRoles {
  parse(accountRoleNumber) {
    let roles = {
      isTrader: ((accountRoleNumber & (1 << AccountRole.TRADER)) != 0),
      isManager: ((accountRoleNumber & (1 << AccountRole.MANAGER)) != 0),
      isService: ((accountRoleNumber & (1 << AccountRole.SERVICE)) != 0),
      isAdmin: ((accountRoleNumber & (1 << AccountRole.ADMINISTRATOR)) != 0)
    };

    return roles;
  }
}

export default new AccountRoles();
