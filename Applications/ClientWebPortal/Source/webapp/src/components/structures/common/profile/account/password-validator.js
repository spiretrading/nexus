class PasswordValidator {
  validate(currentPassword, newPassword, confirmPassword) {
    if (currentPassword == null || currentPassword.length === 0) {
      return "Current password is required.";
    }

    if (newPassword == null || newPassword.length === 0) {
      return "New password is required.";
    }

    if (confirmPassword == null || confirmPassword.length === 0) {
      return "Confirm password is required.";
    }

    if (newPassword !== confirmPassword) {
      return "Confirm password does not match the new password.";
    }

    return null;
  }
}

export default new PasswordValidator();