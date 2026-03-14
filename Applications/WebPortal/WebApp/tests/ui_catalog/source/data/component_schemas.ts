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
      new PropertySchema('theme', WebPortal.Button.Theme.LIGHT,
        EnumInput(WebPortal.Button.Theme)),
      new PropertySchema('disabled', false, BooleanInput),
      new PropertySchema('style', {width: '100%', height: '100%'}, CSSInput)],
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
    [new PropertySchema('checked', true, BooleanInput),
      new PropertySchema('disabled', false, BooleanInput)],
    [new SignalSchema('onClick', 'checked')],
    WebPortal.Checkbox);

const countrySelect =
  new ComponentSchema('CountrySelect',
    [new PropertySchema('value', Nexus.DefaultCountries.US,
        CountryInput),
      new PropertySchema('readOnly', false, BooleanInput),
      new PropertySchema('disabled', false, BooleanInput)],
    [new SignalSchema('onChange', 'value')],
    (props: any) => React.createElement(WebPortal.CountrySelect, {
      ...props,
      countryDatabase: Nexus.defaultCountryDatabase
    }));

const currencyDatabase = Nexus.buildDefaultCurrencyDatabase();

const currencySelect =
  new ComponentSchema('CurrencySelect',
    [new PropertySchema('value', Nexus.DefaultCurrencies.USD,
        CurrencyInput),
      new PropertySchema('readOnly', false, BooleanInput),
      new PropertySchema('disabled', false, BooleanInput)],
    [new SignalSchema('onChange', 'value')],
    (props: any) => React.createElement(WebPortal.CurrencySelect, {
      ...props,
      currencyDatabase
    }));

const dateInput =
  new ComponentSchema('DateInput',
    [new PropertySchema('value',
        Beam.Date.today(), BeamDateInput),
      new PropertySchema('readOnly', false, BooleanInput),
      new PropertySchema('disabled', false, BooleanInput),
      new PropertySchema('error', false, BooleanInput)],
    [new SignalSchema('onChange', 'value')],
    WebPortal.DateInput);

const dateTimeInput =
  new ComponentSchema('DateTimeInput',
    [new PropertySchema('value',
        Beam.DateTime.now(), BeamDateTimeInput),
      new PropertySchema('readOnly', false, BooleanInput),
      new PropertySchema('disabled', false, BooleanInput)],
    [new SignalSchema('onChange', 'value')],
    WebPortal.DateTimeInput);

const dropDownButton =
  new ComponentSchema('DropDownButton',
    [new PropertySchema('size', 16, NumberSliderInput),
      new PropertySchema('isExpanded', false, BooleanInput)],
    [new SignalSchema('onClick', 'isExpanded')],
    (props: any) => React.createElement(WebPortal.DropDownButton, {
      ...props,
      onClick: () => props.onClick(!props.isExpanded)
    }));

const emptyMessage =
  new ComponentSchema('EmptyMessage',
    [new PropertySchema('message', 'No items to display.', TextInput)],
    [],
    WebPortal.EmptyMessage, 732, -1);

const filterInput =
  new ComponentSchema('FilterInput',
    [new PropertySchema('value', '', TextInput),
      new PropertySchema('readOnly', false, BooleanInput),
      new PropertySchema('disabled', false, BooleanInput)],
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
    WebPortal.ErrorMessage, 732, -1);

const durationInput =
  new ComponentSchema('DurationInput',
    [new PropertySchema('value', new Beam.Duration(0), BeamDurationInput),
      new PropertySchema('maxHourValue', 99, NumberInput),
      new PropertySchema('minHourValue', 0, NumberInput),
      new PropertySchema('readOnly', false, BooleanInput),
      new PropertySchema('disabled', false, BooleanInput),
      new PropertySchema('error', false, BooleanInput)],
    [new SignalSchema('onChange', 'value')],
    WebPortal.DurationInput);

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

const input =
  new ComponentSchema('Input',
    [new PropertySchema('value', '', TextInput),
      new PropertySchema('placeholder', 'Enter text...', TextInput),
      new PropertySchema('readOnly', false, BooleanInput),
      new PropertySchema('disabled', false, BooleanInput)],
    [new SignalSchema('onChange', 'value')],
    (props: any) => React.createElement(WebPortal.Input, {
      ...props,
      onChange: (e: any) => props.onChange(e.target.value)
    }));

const integerField =
  new ComponentSchema('IntegerInput',
    [new PropertySchema('min', 0, NumberInput),
      new PropertySchema('max', 100, NumberInput),
      new PropertySchema('value', 0, NumberInput),
      new PropertySchema('readOnly', false, BooleanInput),
      new PropertySchema('disabled', false, BooleanInput),
      new PropertySchema('style', {}, CSSInput)],
    [new SignalSchema('onChange', 'value')],
    WebPortal.IntegerInput);

const labeledCheckbox =
  new ComponentSchema('LabeledCheckbox',
    [new PropertySchema('label', 'Remember me', TextInput),
      new PropertySchema('isChecked', true, BooleanInput)],
    [new SignalSchema('onChange', 'isChecked')],
    WebPortal.LabeledCheckbox);

const modal =
  new ComponentSchema('Modal',
    [new PropertySchema('isOpen', false, BooleanInput),
      new PropertySchema('title', 'Modal Title', TextInput)],
    [new SignalSchema('onClose', 'isOpen')],
    (props: any) => {
      if(!props.isOpen) {
        return React.createElement('div', null, 'Modal is closed.');
      }
      return React.createElement(WebPortal.Modal, {
        title: props.title,
        onClose: () => props.onClose(false)
      },
        React.createElement('div', {
          style: {width: '640px', height: '480px'}
        }));
    });

const decimalInput =
  new ComponentSchema('DecimalInput',
    [new PropertySchema('value', 100, NumberInput),
      new PropertySchema('min', 0, NumberInput),
      new PropertySchema('max', 1000, NumberInput),
      new PropertySchema('decimalPlaces', 2, NumberInput),
      new PropertySchema('readOnly', false, BooleanInput),
      new PropertySchema('disabled', false, BooleanInput)],
    [new SignalSchema('onChange', 'value')],
    WebPortal.DecimalInput);

const moneyInput =
  new ComponentSchema('MoneyInput',
    [new PropertySchema('value', Nexus.Money.parse('100.00'), MoneyInput),
      new PropertySchema('min', Nexus.Money.ZERO, MoneyInput),
      new PropertySchema('max', Nexus.Money.parse('10000.00'), MoneyInput),
      new PropertySchema('decimalPlaces', 2, NumberInput),
      new PropertySchema('readOnly', false, BooleanInput),
      new PropertySchema('disabled', false, BooleanInput),
      new PropertySchema('style', {}, CSSInput)],
    [new SignalSchema('onChange', 'value')],
    WebPortal.MoneyInput);

const NAVIGATION_TABS = [
  React.createElement(WebPortal.NavigationTab, {
    key: 'you',
    icon: 'resources/requests_page/your-requests.svg',
    label: 'Your Requests',
    href: 'requests/you'
  }),
  React.createElement(WebPortal.NavigationTab, {
    key: 'group',
    icon: 'resources/requests_page/group-requests.svg',
    label: 'Group Requests',
    href: 'requests/group'
  }),
  React.createElement(WebPortal.NavigationTab, {
    key: 'approved',
    icon: 'resources/requests_page/approved.svg',
    label: 'Approved',
    href: 'requests/approved'
  })
];

const navigationHeader =
  new ComponentSchema('NavigationHeader',
    [new PropertySchema('current', 'requests/you', TextInput)],
    [new SignalSchema('onNavigate', 'current')],
    (props: any) =>
      React.createElement(WebPortal.NavigationHeader, props,
        ...NAVIGATION_TABS));

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


const pagination =
  new ComponentSchema('Pagination',
    [new PropertySchema('pageIndex', 0, NumberInput),
      new PropertySchema('pageSize', 50, NumberInput),
      new PropertySchema('totalCount', 500, NumberInput)],
    [new SignalSchema('onNavigate', 'pageIndex')],
    WebPortal.Pagination, 800);

const regionInput =
  new ComponentSchema('RegionInput',
    [new PropertySchema('displaySize', WebPortal.DisplaySize.LARGE,
        EnumInput(WebPortal.DisplaySize)),
      new PropertySchema('readOnly', false, BooleanInput),
      new PropertySchema('disabled', false, BooleanInput)],
    [new SignalSchema('onChange', 'value')],
    WebPortal.RegionInput);

const regionItemInput =
  new ComponentSchema('RegionItemInput',
    [new PropertySchema('value', '', TextInput),
      new PropertySchema('readOnly', false, BooleanInput),
      new PropertySchema('disabled', false, BooleanInput)],
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

const select =
  new ComponentSchema('Select',
    [new PropertySchema('value', 'Apple', TextInput),
      new PropertySchema('readOnly', false, BooleanInput),
      new PropertySchema('disabled', false, BooleanInput)],
    [new SignalSchema('onChange', 'value')],
    (props: any) => React.createElement(WebPortal.Select, props,
      React.createElement('option', {value: 'Apple'}, 'Apple'),
      React.createElement('option', {value: 'Banana'}, 'Banana'),
      React.createElement('option', {value: 'Cherry'}, 'Cherry'),
      React.createElement('option', {value: 'Grape'}, 'Grape'),
      React.createElement('option', {value: 'Mango'}, 'Mango'),
      React.createElement('option', {value: 'Orange'}, 'Orange')));

const securitiesInput =
  new ComponentSchema('SecuritiesInput',
    [new PropertySchema('displaySize', WebPortal.DisplaySize.LARGE,
        EnumInput(WebPortal.DisplaySize)),
      new PropertySchema('readOnly', false, BooleanInput),
      new PropertySchema('disabled', false, BooleanInput)],
    [new SignalSchema('onChange', 'value')],
    WebPortal.SecuritiesInput);

const securityInput =
  new ComponentSchema('SecurityInput',
    [new PropertySchema('value', '', TextInput),
      new PropertySchema('readOnly', false, BooleanInput),
      new PropertySchema('disabled', false, BooleanInput)],
    [new SignalSchema('onChange', 'value'),
      new SignalSchema('onEnter', 'value')],
    WebPortal.SecurityInput);

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
      new PropertySchema('readOnly', false, BooleanInput),
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


const timeOfDayInput =
  new ComponentSchema('TimeOfDayInput',
    [new PropertySchema('value', new Beam.Duration(0), BeamTimeOfDayInput),
      new PropertySchema('readOnly', false, BooleanInput),
      new PropertySchema('disabled', false, BooleanInput),
      new PropertySchema('error', false, BooleanInput)],
    [new SignalSchema('onChange', 'value')],
    WebPortal.TimeOfDayInput);

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
      new PropertySchema('direction', WebPortal.RequestsModel.Direction.POSITIVE,
        EnumInput(WebPortal.RequestsModel.Direction))],
    [],
    WebPortal.DiffBadge);

const entitlementsStatusTag =
  new ComponentSchema('EntitlementsStatusTag',
    [new PropertySchema('status',
        WebPortal.RequestsModel.EntitlementStatus.GRANTED,
        EnumInput(WebPortal.RequestsModel.EntitlementStatus))],
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

const CHANGE_TABLE_SAMPLE_DATA: WebPortal.RequestsModel.DetailChange[] = [
  {type: 'entitlement', name: 'NYSE Arca Equities',
    oldStatus: WebPortal.RequestsModel.EntitlementStatus.REVOKED,
    newStatus: WebPortal.RequestsModel.EntitlementStatus.GRANTED,
    delta: {value: '$14.50',
      direction: WebPortal.RequestsModel.Direction.POSITIVE}},
  {type: 'entitlement', name: 'NYSE American Equities',
    oldStatus: WebPortal.RequestsModel.EntitlementStatus.GRANTED,
    newStatus: WebPortal.RequestsModel.EntitlementStatus.REVOKED,
    delta: {value: '$7.00',
      direction: WebPortal.RequestsModel.Direction.NEGATIVE}},
  {type: 'entitlement', name: 'OPRA',
    oldStatus: WebPortal.RequestsModel.EntitlementStatus.REVOKED,
    newStatus: WebPortal.RequestsModel.EntitlementStatus.GRANTED,
    delta: {value: 'FREE',
      direction: WebPortal.RequestsModel.Direction.NONE}},
  {type: 'risk', name: 'Buying Power',
    oldValue: '$100,000.00', newValue: '$150,000.00',
    delta: {value: '$50,000.00',
      direction: WebPortal.RequestsModel.Direction.POSITIVE}},
  {type: 'risk', name: 'Net Loss',
    oldValue: '$5,000.00', newValue: '$3,000.00',
    delta: {value: '$2,000.00',
      direction: WebPortal.RequestsModel.Direction.NEGATIVE}},
  {type: 'risk', name: 'Transition Time',
    oldValue: '1h 00m', newValue: '2h 30m',
    delta: {value: '1h 30m',
      direction: WebPortal.RequestsModel.Direction.POSITIVE}},
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
        WebPortal.RequestsModel.EntitlementAction.GRANT,
        EnumInput(WebPortal.RequestsModel.EntitlementAction)),
      new PropertySchema('fee', Nexus.Money.parse('14.5'), MoneyInput),
      new PropertySchema('direction',
        WebPortal.RequestsModel.Direction.POSITIVE,
        EnumInput(WebPortal.RequestsModel.Direction))],
    [],
    (props: any) => React.createElement(WebPortal.EntitlementsChangeItem, {
      ...props,
      currency: new Nexus.CurrencyDatabase.Entry(
        new Nexus.Currency(840), 'USD', '$')
    }));

const USD_CURRENCY = new Nexus.CurrencyDatabase.Entry(
  new Nexus.Currency(840), 'USD', '$');

const REQUEST_ITEM_SAMPLES: WebPortal.RequestsModel.ListChange[] = [
  {type: 'entitlements', name: 'NYSE Arca Equities',
    action: WebPortal.RequestsModel.EntitlementAction.GRANT,
    fee: Nexus.Money.parse('14.50'), currency: USD_CURRENCY},
  {type: 'entitlements', name: 'OPRA',
    action: WebPortal.RequestsModel.EntitlementAction.REVOKE,
    fee: Nexus.Money.parse('7.00'), currency: USD_CURRENCY},
  {type: 'risk_controls', name: 'Buying Power',
    oldValue: '$100,000', newValue: '$150,000',
    delta: {value: '$50,000',
      direction: WebPortal.RequestsModel.Direction.POSITIVE}},
  {type: 'risk_controls', name: 'Net Loss',
    oldValue: '$5,000', newValue: '$3,000',
    delta: {value: '$2,000',
      direction: WebPortal.RequestsModel.Direction.NEGATIVE}}
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

const requestFilterModal =
  new ComponentSchema('RequestFilterModal',
    [new PropertySchema('isOpen', false, BooleanInput),
      new PropertySchema('displaySize', WebPortal.DisplaySize.LARGE,
        EnumInput(WebPortal.DisplaySize)),
      new PropertySchema('categories',
        new Set([Nexus.AccountModificationRequest.Type.ENTITLEMENTS]),
        ReadonlyInput),
      new PropertySchema('sortKey',
        WebPortal.RequestsModel.SortField.LAST_UPDATED,
        EnumInput(WebPortal.RequestsModel.SortField))],
    [new SignalSchema('onClose', 'isOpen'),
      new SignalSchema('onSubmit', '')],
    (props: any) => {
      if(!props.isOpen) {
        return React.createElement('div', null, 'Modal is closed.');
      }
      return React.createElement(WebPortal.RequestFilterModal, {
        displaySize: props.displaySize,
        categories: props.categories,
        sortKey: props.sortKey,
        onSubmit: props.onSubmit,
        onClose: () => props.onClose(false)
      });
    });

const requestSortSelect =
  new ComponentSchema('RequestSortSelect',
    [new PropertySchema('value',
      WebPortal.RequestsModel.SortField.LAST_UPDATED,
      EnumInput(WebPortal.RequestsModel.SortField))],
    [new SignalSchema('onChange', 'value')],
    WebPortal.RequestSortSelect);

const riskControlsChangeItem =
  new ComponentSchema('RiskControlsChangeItem',
    [new PropertySchema('name', 'Buying Power', TextInput),
      new PropertySchema('oldValue', '$100,000', TextInput),
      new PropertySchema('newValue', '$150,000', TextInput),
      new PropertySchema('delta',
        {value: '$50,000', direction: WebPortal.RequestsModel.Direction.POSITIVE},
        ReadonlyInput)],
    [],
    WebPortal.RiskControlsChangeItem);

const SAMPLE_REQUEST_LIST:
    WebPortal.RequestsModel.RequestEntry[] = (() => {
  const yesterday = new Date(2025, 8, 23);
  return REQUEST_ITEM_SAMPLES.map((change, i) => ({
    id: 1024 + i,
    category: change.type === 'entitlements' ?
      Nexus.AccountModificationRequest.Type.ENTITLEMENTS :
      Nexus.AccountModificationRequest.Type.RISK,
    state: i === 1 ?
      Nexus.AccountModificationRequest.Status.REVIEWED :
      Nexus.AccountModificationRequest.Status.PENDING,
    updateTime: yesterday,
    account: Beam.DirectoryEntry.makeAccount(100, 'achen01'),
    effectiveDate: new Date(2025, 9, 30),
    firstChange: change,
    additionalChangesCount: i === 0 ? 3 : 0,
    commentCount: i === 0 ? 2 : 0,
    managerApproval: i === 1 ?
      {approver: 'cgreen01', self: false} : undefined
  }));
})();

const requestDirectoryPage =
  new ComponentSchema('RequestDirectoryPage',
    [new PropertySchema('displayStatus',
        WebPortal.RequestDirectoryPage.DisplayStatus.READY,
        EnumInput(WebPortal.RequestDirectoryPage.DisplayStatus))],
    [new SignalSchema('onSubmit', '')],
    (props: any) => {
      return React.createElement(WebPortal.RequestDirectoryPage, {
        scope: WebPortal.RequestsModel.Scope.YOU,
        displayStatus: props.displayStatus,
        requestState: WebPortal.RequestsModel.RequestState.PENDING,
        filters: {
          query: '',
          categories: new Set<Nexus.AccountModificationRequest.Type>(),
          sortKey: WebPortal.RequestsModel.SortField.LAST_UPDATED
        },
        filterCount: 0,
        pageIndex: 0,
        response: {
          status: WebPortal.RequestsModel.ResponseStatus.READY,
          facetCounts: {pending: 5, approved: 122, rejected: 122},
          requestList: SAMPLE_REQUEST_LIST
        },
        onSubmit: props.onSubmit
      });
    }, -1);

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
    countrySelect, currencySelect, dateInput,
    dateTimeInput, decimalInput, dropDownButton, durationInput, emptyMessage,
    errorMessage,
    filterChip, filterInput, hLine,
    iconLabelButton, input, integerField, labeledCheckbox, modal, moneyInput,
    navigationHeader, navigationTab, pageLayout,
    pagination, regionInput, regionItemInput, relativeDate, roleIcon, rolePanel,
    securitiesInput, securityInput, select,
    segmentButton,
    segmentedControl,
    timeOfDayInput]),
  new ComponentSection('Requests Page', [accountLink, changeTable,
    complianceRuleStatusTag, diffBadge, entitlementsChangeItem,
    entitlementsStatusTag, requestActivityItem, requestCategoryTag,
    requestDetailPage, requestDirectoryPage, requestEffectiveDate,
    requestFilterModal, requestItem, requestItemPlaceholder,
    requestSortSelect, requestStateIndicator, riskControlsChangeItem])];
