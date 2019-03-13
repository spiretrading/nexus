import * as Beam from 'beam';
import { CountryCode } from '..';

/** Stores identity related info about an account. */
export class AccountIdentity {

  /** Constructs an AccountIdentity from a JSON object. */
  public static fromJson(value: any): AccountIdentity {
    const identity = new AccountIdentity();
    identity.registrationTime = Beam.DateTime.fromJson(value.registration_time);
    identity.lastLoginTime = Beam.DateTime.fromJson(value.last_login_time);
    identity.firstName = value.first_name;
    identity.lastName = value.last_name;
    identity.emailAddress = value.e_mail;
    identity.addressLineOne = value.address_line_one;
    identity.addressLineTwo = value.address_line_two;
    identity.addressLineThree = value.address_line_three;
    identity.city = value.city;
    identity.province = value.province;
    identity.country = CountryCode.fromJson(value.country);
    identity.photoId = value.photo_id;
    identity.userNotes = value.user_notes;
    return identity;
  }

  /** Constructs a default AccountIdentity. */
  constructor() {
    this.registrationTime = Beam.DateTime.NOT_A_DATE_TIME;
    this.lastLoginTime = Beam.DateTime.NOT_A_DATE_TIME;
    this.firstName = '';
    this.lastName = '';
    this.emailAddress = '';
    this.addressLineOne = '';
    this.addressLineTwo = '';
    this.addressLineThree = '';
    this.city = '';
    this.province = '';
    this.country = CountryCode.NONE;
    this.photoId = '';
    this.userNotes = '';
  }

  /** When the account was registered/created. */
  public registrationTime: Beam.DateTime;

  /** The last time the account logged onto the system. */
  public lastLoginTime: Beam.DateTime;

  /** The first name. */
  public firstName: string;

  /** The last name. */
  public lastName: string;
  
  /** The e-mail address. */
  public emailAddress: string;

  /** The first line of the account's address, typically the street. */
  public addressLineOne: string;

  /** The second line of the account's address, such as floor number. */
  public addressLineTwo: string;

  /** The third line of the account's address, used for misc. purposes. */
  public addressLineThree: string;

  /** The city of residence. */
  public city: string;

  /** The province/state/region of residence. */
  public province: string;

  /** The country of residence. */
  public country: CountryCode;

  /** URL to the photo ID image. */
  public photoId: string;

  /** Some additional/misc. user notes. */
  public userNotes: string;

  /** Returns a clone of this identity. */
  public clone(): AccountIdentity {
    const identity = new AccountIdentity();
    identity.registrationTime = this.registrationTime;
    identity.lastLoginTime = this.lastLoginTime;
    identity.firstName = this.firstName;
    identity.lastName = this.lastName;
    identity.emailAddress = this.emailAddress;
    identity.addressLineOne = this.addressLineOne;
    identity.addressLineTwo = this.addressLineTwo;
    identity.addressLineThree = this.addressLineThree;
    identity.city = this.city;
    identity.province = this.province;
    identity.country = this.country;
    identity.photoId = this.photoId;
    identity.userNotes = this.userNotes;
    return identity;
  }

  /** Converts this object to JSON. */
  public toJson(): any {
    return {
      registration_time: this.registrationTime.toJson(),
      last_login_time: this.lastLoginTime.toJson(),
      first_name: this.firstName,
      last_name: this.lastName,
      e_mail: this.emailAddress,
      address_line_one: this.addressLineOne,
      address_line_two: this.addressLineTwo,
      address_line_three: this.addressLineThree,
      city: this.city,
      province: this.province,
      country: this.country.toJson(),
      photo_id: this.photoId,
      user_notes: this.userNotes
    };
  }
}
