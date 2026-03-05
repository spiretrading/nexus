import { StyleSheet } from 'aphrodite';
import * as WebPortal from 'web_portal';
import { ArrayInput, BooleanInput, ColorInput, CSSInput, DateInput, EnumInput,
  NumberInput, NumberSliderInput, OptionalInput, ReadonlyInput,
  StyleDeclarationValueInput, TextInput } from '../viewer/propertyInput';
import {ComponentSchema, PropertySchema, SignalSchema} from './schemas';

const hLine =
  new ComponentSchema('Hline',
    [new PropertySchema('height', 1, NumberSliderInput),
      new PropertySchema('color', '#c2c2c2', ColorInput)],
    [],
    WebPortal.HLine);

const burgerButton =
  new ComponentSchema('BurgerButton',
    [new PropertySchema('width', 26, NumberSliderInput),
      new PropertySchema('height', 20, NumberSliderInput),
      new PropertySchema('color', '#684BC7', ColorInput),
      new PropertySchema('highlightColor', '#684BC7', ColorInput)],
    [new SignalSchema('onClick', '')],
    WebPortal.BurgerButton);

export const componentsList = [burgerButton, hLine];
