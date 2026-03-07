import { css, StyleSheet } from 'aphrodite';
import * as Beam from 'beam';
import * as Nexus from 'nexus';
import * as React from 'react';
import * as WebPortal from 'web_portal';
import { AccountRolesInput, ArrayInput, BeamAccountInput, BeamDateInput,
  BeamDateTimeInput, BeamDurationInput, BeamTimeOfDayInput, BooleanInput,
  ColorInput, CountryInput, CurrencyInput, CSSInput, DateInput, EnumInput,
  NumberInput, NumberSliderInput, OptionalInput, MoneyInput, ReadonlyInput,
  SecurityInput, StyleDeclarationValueInput,
  TextInput } from '../viewer/propertyInput';
import {ComponentSchema, ComponentSection, PropertySchema,
  SignalSchema} from './schemas';

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

const checkbox =
  new ComponentSchema('Checkbox',
    [new PropertySchema('displaySize', WebPortal.DisplaySize.SMALL,
        EnumInput(WebPortal.DisplaySize)),
      new PropertySchema('isChecked', true, BooleanInput),
      new PropertySchema('readonly', false, BooleanInput)],
    [new SignalSchema('onClick', '')],
    WebPortal.Checkbox);

const countrySelectionField =
  new ComponentSchema('CountrySelectionField',
    [new PropertySchema('displaySize', WebPortal.DisplaySize.LARGE,
        EnumInput(WebPortal.DisplaySize)),
      new PropertySchema('value', Nexus.DefaultCountries.US,
        CountryInput),
      new PropertySchema('readonly', false, BooleanInput)],
    [new SignalSchema('onChange', 'value')],
    (props: any) => React.createElement(WebPortal.CountrySelectionField, {
      ...props,
      countryDatabase: Nexus.defaultCountryDatabase
    }));

const currencyDatabase = Nexus.buildDefaultCurrencyDatabase();

const currencySelectionField =
  new ComponentSchema('CurrencySelectionField',
    [new PropertySchema('value', Nexus.DefaultCurrencies.USD,
        CurrencyInput),
      new PropertySchema('readonly', false, BooleanInput)],
    [new SignalSchema('onChange', 'value')],
    (props: any) => React.createElement(WebPortal.CurrencySelectionField, {
      ...props,
      currencyDatabase
    }));

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

const emptyMessage =
  new ComponentSchema('EmptyMessage',
    [new PropertySchema('message', 'No items to display.', TextInput)],
    [],
    WebPortal.EmptyMessage, 640, 480);

const filterInput =
  new ComponentSchema('FilterInput',
    [new PropertySchema('value', '', TextInput)],
    [new SignalSchema('onChange', 'value')],
    WebPortal.FilterInput);

const filterChip =
  new ComponentSchema('FilterChip',
    [new PropertySchema('label', 'Entitlements', TextInput),
      new PropertySchema('isChecked', false, BooleanInput),
      new PropertySchema('disabled', false, BooleanInput)],
    [new SignalSchema('onChange', 'isChecked')],
    WebPortal.FilterChip);

const errorMessage =
  new ComponentSchema('ErrorMessage',
    [new PropertySchema('message',
      'Something went wrong. Please try again later.', TextInput)],
    [new SignalSchema('onRetry', '')],
    WebPortal.ErrorMessage, 640, 480);

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

const labeledCheckbox =
  new ComponentSchema('LabeledCheckbox',
    [new PropertySchema('label', 'Remember me', TextInput),
      new PropertySchema('isChecked', true, BooleanInput)],
    [new SignalSchema('onChange', 'isChecked')],
    WebPortal.LabeledCheckbox);

const modal =
  new ComponentSchema('Modal',
    [new PropertySchema('isOpen', true, BooleanInput),
      new PropertySchema('displaySize', WebPortal.DisplaySize.LARGE,
        EnumInput(WebPortal.DisplaySize)),
      new PropertySchema('title', 'Modal Title', TextInput)],
    [new SignalSchema('onClose', 'isOpen')],
    (props: any) => {
      if(!props.isOpen) {
        return React.createElement('div', null, 'Modal is closed.');
      }
      return React.createElement(WebPortal.Modal, {
        displaySize: props.displaySize,
        title: props.title,
        onClose: () => props.onClose(false)
      },
        React.createElement('div', {
          style: {width: '640px', height: '480px'}
        }));
    });

const moneyField =
  new ComponentSchema('MoneyField',
    [new PropertySchema('value', Nexus.Money.parse('100.00'), MoneyInput),
      new PropertySchema('readonly', false, BooleanInput),
      new PropertySchema('style', {}, CSSInput)],
    [new SignalSchema('onChange', 'value')],
    WebPortal.MoneyField);

const navigationHeader =
  new ComponentSchema('NavigationHeader',
    [],
    [],
    () => React.createElement(WebPortal.NavigationHeader,
      {current: 'requests/you'},
      React.createElement(WebPortal.NavigationTab, {
        icon: 'resources/requests_page/your-requests.svg',
        label: 'Your Requests',
        href: 'requests/you'
      }),
      React.createElement(WebPortal.NavigationTab, {
        icon: 'resources/requests_page/group-requests.svg',
        label: 'Group Requests',
        href: 'requests/group'
      }),
      React.createElement(WebPortal.NavigationTab, {
        icon: 'resources/requests_page/approved.svg',
        label: 'Approved',
        href: 'requests/approved'
      })));

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

const regionField =
  new ComponentSchema('RegionField',
    [new PropertySchema('displaySize', WebPortal.DisplaySize.LARGE,
        EnumInput(WebPortal.DisplaySize)),
      new PropertySchema('readonly', false, BooleanInput)],
    [new SignalSchema('onChange', 'value')],
    WebPortal.RegionField);

const regionItemInput =
  new ComponentSchema('RegionItemInput',
    [new PropertySchema('value', '', TextInput)],
    [new SignalSchema('onChange', 'value'),
      new SignalSchema('onEnter', 'value')],
    WebPortal.RegionItemInput);

const relativeDate =
  new ComponentSchema('RelativeDate',
    [new PropertySchema('datetime', new Date(), DateInput),
      new PropertySchema('today', (() => {
        const now = new Date();
        return new Date(now.getFullYear(), now.getMonth(), now.getDate());
        })(), DateInput)],
    [],
    WebPortal.RelativeDate);

const securitiesField =
  new ComponentSchema('SecuritiesField',
    [new PropertySchema('displaySize', WebPortal.DisplaySize.LARGE,
        EnumInput(WebPortal.DisplaySize)),
      new PropertySchema('readonly', false, BooleanInput)],
    [new SignalSchema('onChange', 'value')],
    WebPortal.SecuritiesField);

const securityFieldInput =
  new ComponentSchema('SecurityInput',
    [new PropertySchema('value', '', TextInput)],
    [new SignalSchema('onChange', 'value'),
      new SignalSchema('onEnter', 'value')],
    WebPortal.SecurityInput);

const securityField =
  new ComponentSchema('SecurityField',
    [new PropertySchema('displaySize', WebPortal.DisplaySize.LARGE,
        EnumInput(WebPortal.DisplaySize)),
      new PropertySchema('value',
        new Nexus.Security('', Nexus.Venue.NONE), SecurityInput),
      new PropertySchema('readonly', false, BooleanInput)],
    [new SignalSchema('onChange', 'value')],
    WebPortal.SecurityField);

const segmentButton =
  new ComponentSchema('SegmentButton',
    [new PropertySchema('name', 'demo-group', TextInput),
      new PropertySchema('label', 'Entitlements', TextInput),
      new PropertySchema('badge', '12', TextInput),
      new PropertySchema('isChecked', false, BooleanInput),
      new PropertySchema('disabled', false, BooleanInput)],
    [new SignalSchema('onChange', '')],
    WebPortal.SegmentButton);

const segmentedControl =
  new ComponentSchema('SegmentedControl',
    [],
    [],
    () => React.createElement(WebPortal.SegmentedControl,
      {name: 'demo-group'},
      React.createElement(WebPortal.SegmentButton,
        {name: '', label: 'Option A', badge: '3', isChecked: true}),
      React.createElement(WebPortal.SegmentButton,
        {name: '', label: 'Option B', badge: '12'}),
      React.createElement(WebPortal.SegmentButton,
        {name: '', label: 'Option C'})));

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

const rolePanel =
  new ComponentSchema('RolePanel',
    [new PropertySchema('roles', new Nexus.AccountRoles(),
        AccountRolesInput)],
    [],
    WebPortal.RolePanel);

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

const CHANGE_TABLE_SAMPLE_DATA: WebPortal.ChangeTable.Change[] = [
  {type: 'entitlement', name: 'NYSE Arca Equities',
    oldStatus: WebPortal.EntitlementsStatusTag.Status.REVOKED,
    newStatus: WebPortal.EntitlementsStatusTag.Status.GRANTED,
    delta: {value: '$14.50',
      direction: WebPortal.DiffBadge.Direction.POSITIVE}},
  {type: 'entitlement', name: 'NYSE American Equities',
    oldStatus: WebPortal.EntitlementsStatusTag.Status.GRANTED,
    newStatus: WebPortal.EntitlementsStatusTag.Status.REVOKED,
    delta: {value: '$7.00',
      direction: WebPortal.DiffBadge.Direction.NEGATIVE}},
  {type: 'entitlement', name: 'OPRA',
    oldStatus: WebPortal.EntitlementsStatusTag.Status.REVOKED,
    newStatus: WebPortal.EntitlementsStatusTag.Status.GRANTED,
    delta: {value: 'FREE',
      direction: WebPortal.DiffBadge.Direction.NONE}},
  {type: 'risk', name: 'Buying Power',
    oldValue: '$100,000.00', newValue: '$150,000.00',
    delta: {value: '$50,000.00',
      direction: WebPortal.DiffBadge.Direction.POSITIVE}},
  {type: 'risk', name: 'Net Loss',
    oldValue: '$5,000.00', newValue: '$3,000.00',
    delta: {value: '$2,000.00',
      direction: WebPortal.DiffBadge.Direction.NEGATIVE}},
  {type: 'risk', name: 'Transition Time',
    oldValue: '1h 00m', newValue: '2h 30m',
    delta: {value: '1h 30m',
      direction: WebPortal.DiffBadge.Direction.POSITIVE}},
  {type: 'risk', name: 'Currency',
    oldValue: 'USD', newValue: 'CAD'}];

const changeTable =
  new ComponentSchema('ChangeTable',
    [],
    [],
    () => React.createElement(WebPortal.ChangeTable,
      {changes: CHANGE_TABLE_SAMPLE_DATA}));

const REQUEST_DETAIL_REQUESTER = {
  account: Beam.DirectoryEntry.makeAccount(201, 'jberrios01'),
  initials: 'JB',
  tint: '#C7BAFF'
};

const REQUEST_DETAIL_APPROVER = {
  account: Beam.DirectoryEntry.makeAccount(305, 'cgreen01'),
  initials: 'CG',
  tint: '#A1F2C1'
};

const REQUEST_DETAIL_ACTIVITY = [
  {account: REQUEST_DETAIL_REQUESTER,
    activity: Nexus.AccountModificationRequest.Status.PENDING,
    timestamp: new Date(2025, 7, 12, 7, 49)},
  {account: REQUEST_DETAIL_REQUESTER,
    activity: 'Need more bp to trade high-value tickers',
    timestamp: new Date(2025, 7, 12, 7, 49)},
  {account: REQUEST_DETAIL_APPROVER,
    activity: Nexus.AccountModificationRequest.Status.REVIEWED,
    timestamp: new Date(2025, 7, 12, 10, 15)},
  {account: REQUEST_DETAIL_APPROVER,
    activity: 'Looks good per performance review',
    timestamp: new Date(2025, 7, 12, 10, 15)}];

const requestDetailPage =
  new ComponentSchema('RequestDetailPage',
    [new PropertySchema('id', 1042, NumberInput),
      new PropertySchema('state',
        Nexus.AccountModificationRequest.Status.PENDING,
        EnumInput(Nexus.AccountModificationRequest.Status)),
      new PropertySchema('createdTime', new Date(2025, 7, 12, 7, 49),
        DateInput),
      new PropertySchema('updateTime', new Date(2025, 7, 13, 10, 15),
        DateInput),
      new PropertySchema('account',
        Beam.DirectoryEntry.makeAccount(142, 'trodriguez'),
        BeamAccountInput),
      new PropertySchema('requester',
        Beam.DirectoryEntry.makeAccount(201, 'jberrios01'),
        BeamAccountInput),
      new PropertySchema('effectiveDate', new Beam.Date(2025, 9, 30),
        BeamDateInput),
      new PropertySchema('accessRole',
        Nexus.AccountRoles.Role.ADMINISTRATOR,
        EnumInput(Nexus.AccountRoles.Role))],
    [new SignalSchema('onApprove', ''),
      new SignalSchema('onReject', '')],
    (props: any) => React.createElement(WebPortal.RequestDetailPage, {
      ...props,
      category: Nexus.AccountModificationRequest.Type.ENTITLEMENTS,
      account: {account: props.account, initials: 'TR', tint: '#FFC880'},
      requester: {account: props.requester, initials: 'JB', tint: '#C7BAFF'},
      changes: CHANGE_TABLE_SAMPLE_DATA,
      activityList: REQUEST_DETAIL_ACTIVITY
    }), -1);

const entitlementsChangeItem =
  new ComponentSchema('EntitlementsChangeItem',
    [new PropertySchema('name', 'NYSE Arca Equities', TextInput),
      new PropertySchema('action',
        WebPortal.EntitlementsChangeItem.Action.GRANT,
        EnumInput(WebPortal.EntitlementsChangeItem.Action)),
      new PropertySchema('fee', Nexus.Money.parse('14.5'), MoneyInput),
      new PropertySchema('direction',
        WebPortal.DiffBadge.Direction.POSITIVE,
        EnumInput(WebPortal.DiffBadge.Direction))],
    [],
    (props: any) => React.createElement(WebPortal.EntitlementsChangeItem, {
      ...props,
      currency: new Nexus.CurrencyDatabase.Entry(
        new Nexus.Currency(840), 'USD', '$')
    }));

const USD_CURRENCY = new Nexus.CurrencyDatabase.Entry(
  new Nexus.Currency(840), 'USD', '$');

const REQUEST_ITEM_SAMPLES: WebPortal.RequestItem.Change[] = [
  {type: 'entitlements', name: 'NYSE Arca Equities',
    action: WebPortal.EntitlementsChangeItem.Action.GRANT,
    fee: Nexus.Money.parse('14.50'), currency: USD_CURRENCY},
  {type: 'entitlements', name: 'OPRA',
    action: WebPortal.EntitlementsChangeItem.Action.REVOKE,
    fee: Nexus.Money.parse('7.00'), currency: USD_CURRENCY},
  {type: 'risk_controls', name: 'Buying Power',
    oldValue: '$100,000', newValue: '$150,000',
    delta: {value: '$50,000',
      direction: WebPortal.DiffBadge.Direction.POSITIVE}},
  {type: 'risk_controls', name: 'Net Loss',
    oldValue: '$5,000', newValue: '$3,000',
    delta: {value: '$2,000',
      direction: WebPortal.DiffBadge.Direction.NEGATIVE}}
];

const requestItem =
  new ComponentSchema('RequestItem',
    [],
    [],
    () => {
      const items: React.ReactElement[] = [];
      const baseDate = new Date(2025, 8, 24);
      const yesterday = new Date(baseDate);
      yesterday.setDate(yesterday.getDate() - 1);
      for(let i = 0; i < REQUEST_ITEM_SAMPLES.length; ++i) {
        const change = REQUEST_ITEM_SAMPLES[i];
        const isEntitlements = change.type === 'entitlements';
        items.push(
          React.createElement(WebPortal.RequestItem, {
            key: i,
            id: 1024 + i,
            category: isEntitlements ?
              Nexus.AccountModificationRequest.Type.ENTITLEMENTS :
              Nexus.AccountModificationRequest.Type.RISK,
            state: i === 1 ?
              Nexus.AccountModificationRequest.Status.REVIEWED :
              Nexus.AccountModificationRequest.Status.PENDING,
            updateTime: yesterday,
            accountName: 'achen01',
            effectiveDate: new Date(2025, 9, 30),
            firstChange: change,
            additionalChangesCount: i === 0 ? 3 : 0,
            commentCount: i === 0 ? 2 : 0,
            managerApproval: i === 1 ?
              {approver: 'cgreen01', self: false} : undefined
          }));
      }
      return React.createElement('div', null, ...items);
    }, -1);

const requestItemPlaceholder =
  new ComponentSchema('RequestItemPlaceholder',
    [],
    [],
    WebPortal.RequestItemPlaceholder, -1);

const requestSortSelect =
  new ComponentSchema('RequestSortSelect',
    [new PropertySchema('value',
      WebPortal.RequestSortSelect.Field.LAST_UPDATED,
      EnumInput(WebPortal.RequestSortSelect.Field))],
    [new SignalSchema('onChange', 'value')],
    WebPortal.RequestSortSelect);

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

const PAGE_LAYOUT_STYLES = StyleSheet.create({
  red: {
    backgroundColor: '#E45532',
    height: '100px'
  },
  green: {
    backgroundColor: '#36B24A',
    height: '150px'
  },
  blue: {
    backgroundColor: '#3366CC',
    height: '80px'
  }
});

const pageLayout =
  new ComponentSchema('PageLayout',
    [],
    [],
    () => React.createElement(WebPortal.PageLayout, null,
      React.createElement('div', null,
        React.createElement('div', {className: css(PAGE_LAYOUT_STYLES.red)}),
        React.createElement('div', {className: css(PAGE_LAYOUT_STYLES.green)}),
        React.createElement('div', {className: css(PAGE_LAYOUT_STYLES.blue)}))),
    -1);

export const componentSections = [
  new ComponentSection('UI Kit', [button, burgerButton, checkbox,
    countrySelectionField, currencySelectionField, dateField,
    dateTimeField, dropDownButton, durationField, emptyMessage, errorMessage,
    filterChip, filterInput, hLine,
    iconLabelButton, integerField, labeledCheckbox, modal, moneyField,
    navigationHeader, navigationTab, numberField, pageLayout,
    pagination, regionField, regionItemInput, relativeDate, roleIcon, rolePanel,
    securitiesField, securityFieldInput, securityField,
    segmentButton,
    segmentedControl, textField,
    timeOfDayField]),
  new ComponentSection('Requests Page', [accountLink, changeTable,
    complianceRuleStatusTag, diffBadge, entitlementsChangeItem,
    entitlementsStatusTag, requestActivityItem, requestCategoryTag,
    requestDetailPage, requestEffectiveDate, requestItem,
    requestItemPlaceholder, requestSortSelect, requestStateIndicator,
    riskControlsChangeItem])];
