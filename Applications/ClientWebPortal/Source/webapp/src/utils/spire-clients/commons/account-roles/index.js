import AccountRole from './account-role';

/** Account roles util class */
class AccountRoles {
  parse(accountRoleNumber) {
    let roles = {
      isTrader: ((accountRoleNumber & Math.pow(2, AccountRole.TRADER)) === Math.pow(2, AccountRole.TRADER)),
      isManager: ((accountRoleNumber & Math.pow(2, AccountRole.MANAGER)) === Math.pow(2, AccountRole.MANAGER)),
      isService: ((accountRoleNumber & Math.pow(2, AccountRole.SERVICE)) === Math.pow(2, AccountRole.SERVICE)),
      isAdmin: ((accountRoleNumber & Math.pow(2, AccountRole.ADMINISTRATOR)) === Math.pow(2, AccountRole.ADMINISTRATOR))
    };

    return roles;
  }
}

export default new AccountRoles();