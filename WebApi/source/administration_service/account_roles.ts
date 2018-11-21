import * as Beam from 'beam';

export class AccountRoles extends Beam.EnumSet<AccountRoles.Role> {}

export namespace AccountRoles {

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
