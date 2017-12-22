import AccountRole from './account-role';

/** Account roles util class */
class AccountRoles {
  constructor(isTrader, isManager, isService, isAdmin) {
    this.isTrader = isTrader;
    this.isManager = isManager;
    this.isService = isService;
    this.isAdmin = isAdmin;
  }

  toData() {
    return AccountRoles.encode(this);
  }
}

AccountRoles.parse = accountRoleNumber => {
  let isTrader = ((accountRoleNumber & (1 << AccountRole.TRADER)) != 0);
  let isManager = ((accountRoleNumber & (1 << AccountRole.MANAGER)) != 0);
  let isService = ((accountRoleNumber & (1 << AccountRole.SERVICE)) != 0);
  let isAdmin = ((accountRoleNumber & (1 << AccountRole.ADMINISTRATOR)) != 0);
  return new AccountRoles(isTrader, isManager, isService, isAdmin);
};

AccountRoles.encode = roles => {
  let rolesNumber = 0;
  if (roles.isTrader) {
    rolesNumber += (1 << AccountRole.TRADER);
  }
  if (roles.isManager) {
    rolesNumber += (1 << AccountRole.MANAGER);
  }
  if (roles.isService) {
    rolesNumber += (1 << AccountRole.SERVICE);
  }
  if (roles.isAdmin) {
    rolesNumber += (1 << AccountRole.ADMINISTRATOR);
  }
  return rolesNumber;
};

export default AccountRoles;
