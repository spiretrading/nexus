import AccountRole from './account-role';

/** Account roles util class */
class AccountRoles {
  parse(accountRoleNumber) {
    let roles = {
      isTrader: ((accountRoleNumber & (1 << AccountRole.TRADER)) === (1 << AccountRole.TRADER)),
      isManager: ((accountRoleNumber & (1 << AccountRole.MANAGER)) === (1 << AccountRole.MANAGER)),
      isService: ((accountRoleNumber & (1 << AccountRole.SERVICE)) === (1 << AccountRole.SERVICE)),
      isAdmin: ((accountRoleNumber & (1 << AccountRole.ADMINISTRATOR)) === (1 << AccountRole.ADMINISTRATOR))
    };

    return roles;
  }
}

export default new AccountRoles();