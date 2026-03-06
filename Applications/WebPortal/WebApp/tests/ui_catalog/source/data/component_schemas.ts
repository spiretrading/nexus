import { StyleSheet } from 'aphrodite';
import * as Beam from 'beam';
import * as Nexus from 'nexus';
import * as WebPortal from 'web_portal';
import { ArrayInput, BeamAccountInput, BeamDateInput, BeamDateTimeInput,
  BeamDurationInput, BeamTimeOfDayInput, BooleanInput, ColorInput, CSSInput,
  DateInput, EnumInput, NumberInput, NumberSliderInput, OptionalInput,
  ReadonlyInput, StyleDeclarationValueInput,
  TextInput } from '../viewer/propertyInput';
import {ComponentSchema, PropertySchema, SignalSchema} from './schemas';

const accountLink =
  new ComponentSchema('AccountLink',
    [new PropertySchema('account',
        Beam.DirectoryEntry.makeAccount(123, 'rileymiller'), BeamAccountInput),
      new PropertySchema('variant', WebPortal.AccountLink.Variant.AVATAR,
        EnumInput(WebPortal.AccountLink.Variant)),
      new PropertySchema('initials', 'RM', TextInput),
      new PropertySchema('tint', '#C7BAFF', ColorInput)],
    [],
    WebPortal.AccountLink);

const button =
  new ComponentSchema('Button',
    [new PropertySchema('label', 'Submit', TextInput),
      new PropertySchema('readonly', false, BooleanInput),
      new PropertySchema('style', {}, CSSInput)],
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
      new PropertySchema('value', (() => {
        const today = new Date();
        return new Beam.Date(
          today.getFullYear(), today.getMonth() + 1, today.getDate());
        })(), BeamDateInput),
      new PropertySchema('readonly', false, BooleanInput)],
    [new SignalSchema('onChange', 'value')],
    WebPortal.DateField);

const dateTimeField =
  new ComponentSchema('DateTimeField',
    [new PropertySchema('displaySize', WebPortal.DisplaySize.SMALL,
        EnumInput(WebPortal.DisplaySize)),
      new PropertySchema('value', (() => {
        const today = new Date();
        return new Beam.DateTime(new Beam.Date(
          today.getFullYear(), today.getMonth() + 1, today.getDate()),
          new Beam.Duration(0));
        })(), BeamDateTimeInput),
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
      new PropertySchema('value', new Beam.Duration(0), BeamDurationInput),
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
    WebPortal.HLine, 100);

const iconLabelButton =
  new ComponentSchema('IconLabelButton',
    [new PropertySchema('icon', 'resources/arrow-next.svg', TextInput),
      new PropertySchema('label', 'Action', TextInput),
      new PropertySchema('variant',
        WebPortal.IconLabelButton.Variant.ICON_LABEL,
        EnumInput(WebPortal.IconLabelButton.Variant)),
      new PropertySchema('iconPlacement',
        WebPortal.IconLabelButton.Placement.LEADING,
        EnumInput(WebPortal.IconLabelButton.Placement)),
      new PropertySchema('disabled', false, BooleanInput)],
    [new SignalSchema('onClick', '')],
    WebPortal.IconLabelButton);

const integerField =
  new ComponentSchema('IntegerField',
    [new PropertySchema('min', 0, NumberInput),
      new PropertySchema('max', 100, NumberInput),
      new PropertySchema('value', 0, NumberInput),
      new PropertySchema('readonly', false, BooleanInput),
      new PropertySchema('style', {}, CSSInput)],
    [new SignalSchema('onChange', 'value')],
    WebPortal.IntegerField);

const navigationTab =
  new ComponentSchema('NavigationTab',
    [new PropertySchema('icon', 'resources/requests_page/your-requests.svg',
        TextInput),
      new PropertySchema('label', 'Your Requests', TextInput),
      new PropertySchema('href', 'requests/you', TextInput),
      new PropertySchema('isCurrent', true, BooleanInput),
      new PropertySchema('variant',
        WebPortal.NavigationTab.Variant.ICON_LABEL,
        EnumInput(WebPortal.NavigationTab.Variant))],
    [new SignalSchema('onClick', '')],
    WebPortal.NavigationTab);

const numberField =
  new ComponentSchema('NumberField',
    [new PropertySchema('value', 0, NumberInput),
      new PropertySchema('min', 0, NumberInput),
      new PropertySchema('max', 100, NumberInput),
      new PropertySchema('readonly', false, BooleanInput),
      new PropertySchema('style', {}, CSSInput)],
    [new SignalSchema('onChange', 'value')],
    WebPortal.NumberField);

const pagination =
  new ComponentSchema('Pagination',
    [new PropertySchema('pageIndex', 0, NumberInput),
      new PropertySchema('pageSize', 50, NumberInput),
      new PropertySchema('totalCount', 500, NumberInput)],
    [new SignalSchema('onNavigate', 'pageIndex')],
    WebPortal.Pagination, 800);

const relativeDate =
  new ComponentSchema('RelativeDate',
    [new PropertySchema('datetime', new Date(), DateInput),
      new PropertySchema('today', (() => {
        const now = new Date();
        return new Date(now.getFullYear(), now.getMonth(), now.getDate());
        })(), DateInput)],
    [],
    WebPortal.RelativeDate);

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
      new PropertySchema('readonly', false, BooleanInput),
      new PropertySchema('style', {}, CSSInput)],
    [new SignalSchema('onInput', 'value')],
    WebPortal.TextField);

const timeOfDayField =
  new ComponentSchema('TimeOfDayField',
    [new PropertySchema('displaySize', WebPortal.DisplaySize.SMALL,
        EnumInput(WebPortal.DisplaySize)),
      new PropertySchema('value', new Beam.Duration(0), BeamTimeOfDayInput),
      new PropertySchema('readonly', false, BooleanInput)],
    [new SignalSchema('onChange', 'value')],
    WebPortal.TimeOfDayField);

const complianceRuleStatusTag =
  new ComponentSchema('ComplianceRuleStatusTag',
    [new PropertySchema('status',
        Nexus.ComplianceRuleEntry.State.ACTIVE,
        EnumInput(Nexus.ComplianceRuleEntry.State))],
    [],
    WebPortal.ComplianceRuleStatusTag);

const diffBadge =
  new ComponentSchema('DiffBadge',
    [new PropertySchema('value', '3', TextInput),
      new PropertySchema('direction', WebPortal.DiffBadge.Direction.POSITIVE,
        EnumInput(WebPortal.DiffBadge.Direction))],
    [],
    WebPortal.DiffBadge);

const entitlementsStatusTag =
  new ComponentSchema('EntitlementsStatusTag',
    [new PropertySchema('status',
        WebPortal.EntitlementsStatusTag.Status.GRANTED,
        EnumInput(WebPortal.EntitlementsStatusTag.Status))],
    [],
    WebPortal.EntitlementsStatusTag);

const requestCategoryTag =
  new ComponentSchema('RequestCategoryTag',
    [new PropertySchema('category',
        Nexus.AccountModificationRequest.Type.RISK,
        EnumInput(Nexus.AccountModificationRequest.Type))],
    [],
    WebPortal.RequestCategoryTag);

const requestActivityItem =
  new ComponentSchema('RequestActivityItem',
    [new PropertySchema('account',
        Beam.DirectoryEntry.makeAccount(123, 'rileymiller'), BeamAccountInput),
      new PropertySchema('activity',
        Nexus.AccountModificationRequest.Status.PENDING,
        EnumInput(Nexus.AccountModificationRequest.Status)),
      new PropertySchema('timestamp', new Date(), DateInput),
      new PropertySchema('initials', 'RM', TextInput),
      new PropertySchema('tint', '#C7BAFF', ColorInput)],
    [],
    WebPortal.RequestActivityItem);

const requestEffectiveDate =
  new ComponentSchema('RequestEffectiveDate',
    [new PropertySchema('date', new Date('2026-04-15T00:00:00'), DateInput),
      new PropertySchema('isApproved', false, BooleanInput),
      new PropertySchema('today', new Date('2026-03-04T00:00:00'), DateInput)],
    [],
    WebPortal.RequestEffectiveDate);

const requestStateIndicator =
  new ComponentSchema('RequestStateIndicator',
    [new PropertySchema('state',
        Nexus.AccountModificationRequest.Status.PENDING,
        EnumInput(Nexus.AccountModificationRequest.Status))],
    [],
    WebPortal.RequestStateIndicator);

const changeTable =
  new ComponentSchema('ChangeTable',
    [new PropertySchema('changes',
        [{name: 'NYSE Arca Equities', oldStatus:
            WebPortal.EntitlementsStatusTag.Status.REVOKED, newStatus:
            WebPortal.EntitlementsStatusTag.Status.GRANTED, delta:
            {value: '$14.50', direction:
              WebPortal.DiffBadge.Direction.POSITIVE}},
          {name: 'NYSE American Equities', oldStatus:
            WebPortal.EntitlementsStatusTag.Status.GRANTED, newStatus:
            WebPortal.EntitlementsStatusTag.Status.REVOKED, delta:
            {value: '$7.00', direction:
              WebPortal.DiffBadge.Direction.NEGATIVE}},
          {name: 'OPRA', oldStatus:
            WebPortal.EntitlementsStatusTag.Status.GRANTED, newStatus:
            WebPortal.EntitlementsStatusTag.Status.GRANTED, delta:
            {value: '$0.00', direction:
              WebPortal.DiffBadge.Direction.NONE}}] as
        WebPortal.ChangeTable.Change[], ReadonlyInput)],
    [],
    WebPortal.ChangeTable);

const riskControlsChangeItem =
  new ComponentSchema('RiskControlsChangeItem',
    [new PropertySchema('name', 'Buying Power', TextInput),
      new PropertySchema('oldValue', '$100,000', TextInput),
      new PropertySchema('newValue', '$150,000', TextInput),
      new PropertySchema('delta',
        {value: '$50,000', direction: WebPortal.DiffBadge.Direction.POSITIVE},
        ReadonlyInput)],
    [],
    WebPortal.RiskControlsChangeItem);

export const componentsList = [accountLink, button, burgerButton, changeTable,
  checkmark, complianceRuleStatusTag, dateField, dateTimeField, diffBadge,
  dropDownButton, durationField, entitlementsStatusTag, hLine,
  iconLabelButton, integerField, navigationTab, numberField, pagination,
  relativeDate, requestActivityItem, requestCategoryTag, requestEffectiveDate,
  requestStateIndicator, riskControlsChangeItem, roleIcon, textField,
  timeOfDayField];
