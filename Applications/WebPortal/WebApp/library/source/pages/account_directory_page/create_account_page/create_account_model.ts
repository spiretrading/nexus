import * as Beam from 'beam';
import * as Nexus from 'nexus';

export abstract class CreateAccountModel {

  /** The maximum length of an account's username. */
  public static readonly MAX_USERNAME_LENGTH = 100;

  /** Returns the canonical form of a username.
   * @param username - The username to normalize.
   * @return The username without surrounding whitespace and in lower case.
   */
  public static normalizeUsername(username: string): string {
    return username.trim().toLowerCase();
  }

  /** Returns the error in the format of a normalized username.
   * @param username - The normalized username to check.
   * @return The validation error, or NONE if the format is valid.
   */
  public static checkUsernameFormat(username: string):
      CreateAccountModel.ValidationError {
    if(username === '') {
      return CreateAccountModel.ValidationError.REQUIRED;
    }
    if(username.length > CreateAccountModel.MAX_USERNAME_LENGTH ||
        !CreateAccountModel.USERNAME_PATTERN.test(username)) {
      return CreateAccountModel.ValidationError.FORMAT;
    }
    return CreateAccountModel.ValidationError.NONE;
  }

  /**
   * Submits information to create an account.
   * @param username - The username of the new account.
   * @param groups - The groups associated with the new account.
   * @param identity - The identity of the new account.
   * @param roles - The roles associated with the new account.
   */
  public abstract createAccount(username: string, 
    groups: Beam.DirectoryEntry, identity: Nexus.AccountIdentity,
    roles: Nexus.AccountRoles): Promise<void>;

  /**
   * Returns the error in a username, if any.
   * @param username - The username to validate.
   * @return The validation error, or NONE if the username may be used.
   */
  public abstract validateUsername(username: string):
    Promise<CreateAccountModel.ValidationError>;

  private static readonly USERNAME_PATTERN =
    /^[a-z0-9]([a-z0-9._-]*[a-z0-9])?$/;
}

export namespace CreateAccountModel {

  /** Lists the reasons an input's value can be rejected. */
  export enum ValidationError {

    /** The value is valid. */
    NONE,

    /** The value is empty where one is required. */
    REQUIRED,

    /** The value is already in use. */
    DUPLICATE,

    /** The value contains invalid characters. */
    FORMAT
  }
}
