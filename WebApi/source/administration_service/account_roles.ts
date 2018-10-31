/** Stores a set of account roles.*/
export class AccountRoles {

  /** Constructs a set of AccountRoles from a JSON object. */
  public static fromJson(object: any) {
    return new AccountRoles(object);
  }

  /** Constructs a set of AccountRoles.
   * @param value - The bitset storing the roles.
   */
  constructor(value: number = 0) {
    this._value = value;
  }

  /** Tests if a role is a member of this set.
   * @param role - The role to test.
   * @return - true iff the role is a member of the set.
   */
  public isSet(role: AccountRoles.Role): boolean {
    if(role === AccountRoles.Role.NONE) {
      return true;
    }
    return (this._value & (1 << (role - 1))) != 0;
  }

  private _value: number;
}

export module AccountRoles {

  /** Enumerates the available account roles. */
  export enum Role {

    /** The account has no roles. */
    NONE = 0,

    /** The account has permission to trade. */
    TRADER,

    /** The account has permission to manage traders. */
    MANAGER,

    /** The account provides API services. */
    SERVICE,

    /** The account is an administrator. */
    ADMINISTRATOR
  }
}
