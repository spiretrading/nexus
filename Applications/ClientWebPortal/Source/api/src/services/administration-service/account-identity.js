class AccountEntity {
  constructor(
    addressLineOne,
    addressLineTwo,
    addressLineThree,
    city,
    country,
    email,
    firstName,
    lastLoginTime,
    lastName,
    picture,
    province,
    registrationTime,
    userNotes
  ) {
    this.addressLineOne = addressLineOne;
    this.addressLineTwo = addressLineTwo;
    this.addressLineThree = addressLineThree;
    this.city = city;
    this.country = country;
    this.email = email;
    this.firstName = firstName;
    this.lastLoginTime = lastLoginTime || '00000000T000000';
    this.lastName = lastName;
    this.picture = picture;
    this.province = province;
    this.registrationTime = registrationTime || '00000000T000000';
    this.userNotes = userNotes;
  }

  toData() {
    return {
      address_line_one: this.addressLineOne,
      address_line_two: this.addressLineTwo,
      address_line_three: this.addressLineThree,
      city: this.city,
      country: this.country,
      e_mail: this.email,
      first_name: this.firstName,
      last_login_time: this.lastLoginTime,
      last_name: this.lastName,
      photo_id: this.picture,
      province: this.province,
      registration_time: this.registrationTime,
      user_notes: this.userNotes
    };
  }
}

AccountEntity.fromData = (data) => {
  let accountEntity = new AccountEntity();
  accountEntity.addressLineOne = data.address_line_one;
  accountEntity.addressLineTwo = data.address_line_two;
  accountEntity.addressLineThree = data.address_line_three;
  accountEntity.city = data.city;
  accountEntity.country = data.country;
  accountEntity.email = data.e_mail;
  accountEntity.firstName = data.first_name;
  accountEntity.lastLoginTime = data.last_login_time;
  accountEntity.lastName = data.last_name;
  accountEntity.picture = data.photo_id;
  accountEntity.province = data.province;
  accountEntity.registrationTime = data.registration_time;
  accountEntity.userNotes = data.user_notes;
  return accountEntity;
};

export default AccountEntity;
