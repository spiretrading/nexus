import { StyleSheet } from 'aphrodite';
import * as Beam from 'beam';
import * as Nexus from 'nexus';
import * as WebPortal from 'web_portal';
import { ArrayInput, BooleanInput, ColorInput, CSSInput, DateInput, EnumInput,
  NumberInput, NumberSliderInput, OptionalInput, ReadonlyInput,
  StyleDeclarationValueInput, TextInput } from '../viewer/propertyInput';
import {ComponentSchema, PropertySchema, SignalSchema} from './schemas';

const button =
  new ComponentSchema('Button',
    [new PropertySchema('label', 'Submit', TextInput),
      new PropertySchema('readonly', false, BooleanInput)],
    [new SignalSchema('onClick', '')],
    WebPortal.Button);

const burgerButton =
  new ComponentSchema('BurgerButton',
    [new PropertySchema('width', 26, NumberSliderInput),
      new PropertySchema('height', 20, NumberSliderInput),
      new PropertySchema('color', '#684BC7', ColorInput),
      new PropertySchema('highlightColor', '#684BC7', ColorInput)],
    [new SignalSchema('onClick', '')],
    WebPortal.BurgerButton);

const checkmark =
  new ComponentSchema('Checkmark',
    [new PropertySchema('displaySize', WebPortal.DisplaySize.SMALL,
        EnumInput(WebPortal.DisplaySize)),
      new PropertySchema('isChecked', true, BooleanInput),
      new PropertySchema('readonly', false, BooleanInput)],
    [new SignalSchema('onClick', '')],
    WebPortal.Checkmark);

const dateField =
  new ComponentSchema('DateField',
    [new PropertySchema('displaySize', WebPortal.DisplaySize.SMALL,
        EnumInput(WebPortal.DisplaySize)),
      new PropertySchema('value', new Beam.Date(1, 1, 2026), ReadonlyInput),
      new PropertySchema('readonly', false, BooleanInput)],
    [new SignalSchema('onChange', 'value')],
    WebPortal.DateField);

const dateTimeField =
  new ComponentSchema('DateTimeField',
    [new PropertySchema('displaySize', WebPortal.DisplaySize.SMALL,
        EnumInput(WebPortal.DisplaySize)),
      new PropertySchema('value',
        new Beam.DateTime(new Beam.Date(1, 1, 2026), new Beam.Duration(0)),
        ReadonlyInput),
      new PropertySchema('readonly', false, BooleanInput)],
    [new SignalSchema('onChange', 'value')],
    WebPortal.DateTimeField);

const dropDownButton =
  new ComponentSchema('DropDownButton',
    [new PropertySchema('size', 16, NumberSliderInput),
      new PropertySchema('isExpanded', false, BooleanInput)],
    [new SignalSchema('onClick', '')],
    WebPortal.DropDownButton);

const durationField =
  new ComponentSchema('DurationField',
    [new PropertySchema('displaySize', WebPortal.DisplaySize.SMALL,
        EnumInput(WebPortal.DisplaySize)),
      new PropertySchema('value', new Beam.Duration(0), ReadonlyInput),
      new PropertySchema('maxHourValue', 99, NumberInput),
      new PropertySchema('minHourValue', 0, NumberInput),
      new PropertySchema('readonly', false, BooleanInput)],
    [new SignalSchema('onChange', 'value')],
    WebPortal.DurationField);

const hLine =
  new ComponentSchema('Hline',
    [new PropertySchema('height', 1, NumberSliderInput),
      new PropertySchema('color', '#c2c2c2', ColorInput)],
    [],
    WebPortal.HLine);

const integerField =
  new ComponentSchema('IntegerField',
    [new PropertySchema('min', 0, NumberInput),
      new PropertySchema('max', 100, NumberInput),
      new PropertySchema('value', 0, NumberInput),
      new PropertySchema('readonly', false, BooleanInput)],
    [new SignalSchema('onChange', 'value')],
    WebPortal.IntegerField);

const modal =
  new ComponentSchema('Modal',
    [new PropertySchema('displaySize', WebPortal.DisplaySize.LARGE,
        EnumInput(WebPortal.DisplaySize)),
      new PropertySchema('height', '400px', TextInput),
      new PropertySchema('width', '600px', TextInput),
      new PropertySchema('children', 'Modal Content', TextInput)],
    [new SignalSchema('onClose', '')],
    WebPortal.Modal);

const numberField =
  new ComponentSchema('NumberField',
    [new PropertySchema('value', 0, NumberInput),
      new PropertySchema('min', 0, NumberInput),
      new PropertySchema('max', 100, NumberInput),
      new PropertySchema('readonly', false, BooleanInput)],
    [new SignalSchema('onChange', 'value')],
    WebPortal.NumberField);

const roleIcon =
  new ComponentSchema('RoleIcon',
    [new PropertySchema('displaySize', WebPortal.DisplaySize.SMALL,
        EnumInput(WebPortal.DisplaySize)),
      new PropertySchema('isExtraSmall', false, BooleanInput),
      new PropertySchema('role', Nexus.AccountRoles.Role.TRADER,
        EnumInput(Nexus.AccountRoles.Role)),
      new PropertySchema('readonly', false, BooleanInput),
      new PropertySchema('isSet', true, BooleanInput),
      new PropertySchema('isTouchTooltipShown', false, BooleanInput)],
    [new SignalSchema('onClick', ''),
      new SignalSchema('onTouch', '')],
    WebPortal.RoleIcon);

const textField =
  new ComponentSchema('TextField',
    [new PropertySchema('displaySize', WebPortal.DisplaySize.SMALL,
        EnumInput(WebPortal.DisplaySize)),
      new PropertySchema('value', 'Hello World', TextInput),
      new PropertySchema('placeholder', 'Enter text', TextInput),
      new PropertySchema('isError', false, BooleanInput),
      new PropertySchema('readonly', false, BooleanInput)],
    [new SignalSchema('onInput', 'value')],
    WebPortal.TextField);

const timeOfDayField =
  new ComponentSchema('TimeOfDayField',
    [new PropertySchema('displaySize', WebPortal.DisplaySize.SMALL,
        EnumInput(WebPortal.DisplaySize)),
      new PropertySchema('value', new Beam.Duration(0), ReadonlyInput),
      new PropertySchema('readonly', false, BooleanInput)],
    [new SignalSchema('onChange', 'value')],
    WebPortal.TimeOfDayField);

export const componentsList = [button, burgerButton, checkmark, dateField,
  dateTimeField, dropDownButton, durationField, hLine, integerField, modal,
  numberField, roleIcon, textField, timeOfDayField];
