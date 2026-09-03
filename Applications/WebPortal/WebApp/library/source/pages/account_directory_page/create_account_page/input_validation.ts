import { CreateAccountModel } from './create_account_model';

/** Stores the validation state of a single input. */
export interface InputValidation {

  /** Whether the input's value is valid. */
  valid: boolean;

  /** The validation error, if any. */
  error: CreateAccountModel.ValidationError;

  /** Whether the error is displayed. */
  showError: boolean;
}

/** The validation state of an input whose value has yet to be entered. */
export const VALID_INPUT: InputValidation = {
  valid: true,
  error: CreateAccountModel.ValidationError.NONE,
  showError: false
};
