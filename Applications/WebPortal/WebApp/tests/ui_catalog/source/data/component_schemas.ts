import { css, StyleSheet } from 'aphrodite';
import * as Beam from 'beam';
import * as Nexus from 'nexus';
import * as React from 'react';
import * as WebPortal from 'web_portal';
import { AccountRolesInput, ArrayInput, BeamAccountInput, BeamDateInput,
  BeamDateTimeInput, BeamDirectoryEntryInput, BeamDurationInput,
  BeamTimeOfDayInput, BooleanInput, ColorInput, CountryInput, CurrencyInput,
  CSSInput, DateInput, DateRangeValueInput, EnumInput, IntervalValueInput,
  NumberInput,
  NumberSliderInput, MoneyInput,
  ReadonlyInput, ScopeValueInput, TextInput, TickerValueInput } from
  '../viewer/propertyInput';
import { AccountGroupListInputExample } from
  './account_group_list_input_example';
import { ActivityTableRowExample } from './activity_table_row_example';
import { ActivityTableRowPlaceholderExample } from
  './activity_table_row_placeholder_example';
import { DateRangeInputExample } from './date_range_input_example';
import { ListInputExample } from './list_input_example';
import { InputGroupExample } from './input_group_example';
import { RadioButtonExample } from './radio_button_example';
import { ReportTableExample } from './report_table_example';
import { ReportTableRowExample } from './report_table_row_example';
import { ReportTableRowPlaceholderExample } from
  './report_table_row_placeholder_example';
import { ScheduledReportItemExample, ScheduledReportParameterInput } from
  './scheduled_report_item_example';
import { ScheduledReportItemContextMenuExample } from
  './scheduled_report_item_context_menu_example';
import { ScopeInputExample } from './scope_input_example';
import { ShareReportModalExample } from './share_report_modal_example';
import { TickerInputExample } from './ticker_input_example';
import {ComponentSchema, ComponentSection, PropertySchema, SignalSchema} from
  './schemas';

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
      new PropertySchema('variant', WebPortal.Button.Variant.PRIMARY,
        EnumInput(WebPortal.Button.Variant)),
      new PropertySchema('disabled', false, BooleanInput),
      new PropertySchema('style', {}, CSSInput)],
    [new SignalSchema('onClick', '')],
    WebPortal.Button);

const buttonLink =
  new ComponentSchema('ButtonLink',
    [new PropertySchema('label', 'View report', TextInput),
      new PropertySchema('href', '#report', TextInput),
      new PropertySchema('target', '_self', TextInput),
      new PropertySchema('inert', false, BooleanInput),
      new PropertySchema('style', {}, CSSInput)],
    [new SignalSchema('onClick', '')],
    WebPortal.ButtonLink);

const burgerButton =
  new ComponentSchema('BurgerButton',
    [new PropertySchema('width', 26, NumberSliderInput),
      new PropertySchema('height', 20, NumberSliderInput),
      new PropertySchema('color', '#684BC7', ColorInput),
      new PropertySchema('highlightColor', '#684BC7', ColorInput),
      new PropertySchema('disabled', false, BooleanInput)],
    [new SignalSchema('onClick', '')],
    WebPortal.BurgerButton);

const checkbox =
  new ComponentSchema('Checkbox',
    [new PropertySchema('checked', true, BooleanInput),
      new PropertySchema('indeterminate', false, BooleanInput),
      new PropertySchema('disabled', false, BooleanInput)],
    [new SignalSchema('onClick', 'checked')],
    (props: any) => {
      const ref = React.useCallback((node: HTMLDivElement) => {
        if(node) {
          const input = node.querySelector('input');
          if(input) {
            input.indeterminate = props.indeterminate;
          }
        }
      }, [props.indeterminate]);
      return React.createElement('div', {ref: ref},
        React.createElement(WebPortal.Checkbox, {
          checked: props.checked,
          disabled: props.disabled,
          onClick: props.onClick
        }));
    });

const radioButton =
  new ComponentSchema('RadioButton',
    [new PropertySchema('label', 'Now', TextInput),
      new PropertySchema('name', 'catalog-runtime', TextInput),
      new PropertySchema('checked', true, BooleanInput),
      new PropertySchema('disabled', false, BooleanInput)],
    [new SignalSchema('onChange', 'checked')],
    RadioButtonExample);

enum MenuMode {
  COMMANDS,
  RADIO_GROUP,
  MIXED,
  SCROLLING
}

const makeMenuItem = (label: string, value: string) => ({
  body: React.createElement(WebPortal.ContextMenu.Item, {label: label}),
  value: value
});

const makeMenuItemRadio = (label: string, group: string, value: string,
    checked?: boolean) => ({
  body: React.createElement(WebPortal.ContextMenu.ItemRadio,
    {label: label, group: group, checked: checked}),
  value: value
});

const makeMenuSeparator = () => ({
  body: React.createElement(WebPortal.ContextMenu.Separator)
});

const CONTEXT_MENU_ITEMS: Record<MenuMode, WebPortal.ContextMenu.Entry[]> = {
  [MenuMode.COMMANDS]: [
    makeMenuItem('Full Width', 'full-width'),
    makeMenuItem('Duplicate Widget', 'duplicate-widget'),
    makeMenuSeparator(),
    makeMenuItem('Remove Widget', 'remove-widget')],
  [MenuMode.RADIO_GROUP]: [
    makeMenuItemRadio('Small', 'size', 'small'),
    makeMenuItemRadio('Medium', 'size', 'medium', true),
    makeMenuItemRadio('Large', 'size', 'large')],
  [MenuMode.MIXED]: [
    makeMenuItem('Full Width', 'full-width'),
    makeMenuSeparator(),
    makeMenuItemRadio('Small', 'size', 'small'),
    makeMenuItemRadio('Medium', 'size', 'medium', true),
    makeMenuItemRadio('Large', 'size', 'large'),
    makeMenuSeparator(),
    makeMenuItem('Remove Widget', 'remove-widget')],
  [MenuMode.SCROLLING]: Array.from({length: 30}, (element, index) =>
    makeMenuItem(`Command ${index + 1}`, `command-${index + 1}`))
};

const CONTEXT_MENU_STYLES = StyleSheet.create({
  invoker: {
    anchorName: '--catalog-context-menu'
  },
  contextMenu: {
    positionAnchor: '--catalog-context-menu',
    positionArea: 'bottom span-right',
    positionTryFallbacks: 'flip-block flip-inline',
    margin: '2px 0'
  }
});

class ContextMenuDemo extends React.Component<any> {
  constructor(props: any) {
    super(props);
    this.invoker = React.createRef();
  }

  public render(): JSX.Element {
    return React.createElement('div', null,
      React.createElement('button', {
        ref: this.invoker,
        popovertarget: 'catalog-context-menu',
        className: css(CONTEXT_MENU_STYLES.invoker)
      } as any, 'Open Menu'),
      React.createElement(WebPortal.ContextMenu, {
        id: 'catalog-context-menu',
        invoker: this.invoker,
        items: CONTEXT_MENU_ITEMS[this.props.mode as MenuMode],
        className: css(CONTEXT_MENU_STYLES.contextMenu),
        onSubmit: this.props.onSubmit
      }));
  }

  private invoker: React.RefObject<HTMLButtonElement>;
}

const contextMenu =
  new ComponentSchema('ContextMenu',
    [new PropertySchema('mode', MenuMode.MIXED, EnumInput(MenuMode)),
      new PropertySchema('lastSubmitted', '', TextInput)],
    [new SignalSchema('onSubmit', 'lastSubmitted')],
    ContextMenuDemo);

const countrySelect =
  new ComponentSchema('CountrySelect',
    [new PropertySchema('value', Nexus.Countries.US,
        CountryInput),
      new PropertySchema('readOnly', false, BooleanInput),
      new PropertySchema('disabled', false, BooleanInput)],
    [new SignalSchema('onChange', 'value')],
    (props: any) => React.createElement(WebPortal.CountrySelect, {
      ...props,
      countryDatabase: Nexus.countryDatabase,
      style: {width: '100%'}
    }));

const currencyDatabase = Nexus.buildCurrencyDatabase();

const currencySelect =
  new ComponentSchema('CurrencySelect',
    [new PropertySchema('value', Nexus.Currencies.USD,
        CurrencyInput),
      new PropertySchema('readOnly', false, BooleanInput),
      new PropertySchema('disabled', false, BooleanInput)],
    [new SignalSchema('onChange', 'value')],
    (props: any) => React.createElement(WebPortal.CurrencySelect, {
      ...props,
      currencyDatabase,
      style: {width: '100%'}
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

const disclosure =
  new ComponentSchema('Disclosure',
    [new PropertySchema('open', false, BooleanInput)],
    [new SignalSchema('onToggle', 'open')],
    (props: any) =>
      React.createElement(WebPortal.Disclosure, {
        ...props,
        header: React.createElement('div', {
          style: {
            display: 'flex', alignItems: 'center', gap: '18px',
            height: '40px', fontSize: '0.875rem'
          }
        },
          React.createElement(WebPortal.ExpandButton, {
            isExpanded: props.open, size: '20'
          }),
          'Section Title'),
        details: React.createElement('div', {
          style: {padding: '12px', fontSize: '0.875rem'}
        }, 'This is the details content that is visible when the disclosure is open.')
      }));

const dateTimeInput =
  new ComponentSchema('DateTimeInput',
    [new PropertySchema('value',
        Beam.DateTime.now(), BeamDateTimeInput),
      new PropertySchema('readOnly', false, BooleanInput),
      new PropertySchema('disabled', false, BooleanInput)],
    [new SignalSchema('onChange', 'value')],
    WebPortal.DateTimeInput);

const expandButton =
  new ComponentSchema('ExpandButton',
    [new PropertySchema('size', 16, NumberSliderInput),
      new PropertySchema('isExpanded', false, BooleanInput)],
    [new SignalSchema('onClick', 'isExpanded')],
    (props: any) => React.createElement(WebPortal.ExpandButton, {
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

const iconButton =
  new ComponentSchema('IconButton',
    [new PropertySchema('icon', 'data:image/svg+xml,' + encodeURIComponent(
        '<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 16 16">' +
        '<path d="M12 8L5.70703 13.5L4 11.6738L8.22396 8L4 4.32617' +
        'L5.70703 2.5L12 8Z"/></svg>'), TextInput),
      new PropertySchema('aria-label', 'Next', TextInput),
      new PropertySchema('disabled', false, BooleanInput),
      new PropertySchema('style', {}, CSSInput)],
    [new SignalSchema('onClick', '')],
    WebPortal.IconButton);

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
      style: {width: '100%', ...props.style},
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
    (props: any) => React.createElement(WebPortal.IntegerInput,
      {...props, style: {width: '100%', ...props.style}}));

const labeledCheckbox =
  new ComponentSchema('LabeledCheckbox',
    [new PropertySchema('label', 'Remember me', TextInput),
      new PropertySchema('isChecked', true, BooleanInput)],
    [new SignalSchema('onChange', 'isChecked')],
    WebPortal.LabeledCheckbox);

const link =
  new ComponentSchema('Link',
    [new PropertySchema('label', 'Learn more', TextInput),
      new PropertySchema('href', '#', TextInput)],
    [new SignalSchema('onClick', '')],
    WebPortal.Link);

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

const accountGroupListInput =
  new ComponentSchema('AccountGroupListInput',
    [new PropertySchema('value',
        [Beam.DirectoryEntry.makeAccount(1, 'Alice')],
        ArrayInput(new PropertySchema('entry',
          Beam.DirectoryEntry.makeAccount(1, 'Alice'),
          BeamDirectoryEntryInput))),
      new PropertySchema('readOnly', false, BooleanInput),
      new PropertySchema('disabled', false, BooleanInput),
      new PropertySchema('lookupDelay', 300, NumberInput),
      new PropertySchema('failLookup', false, BooleanInput)],
    [new SignalSchema('onChange', 'value')],
    AccountGroupListInputExample, 300);

const listInput =
  new ComponentSchema('ListInput',
    [new PropertySchema('title', 'Edit Items', TextInput),
      new PropertySchema('listHeading', 'Added Items', TextInput),
      new PropertySchema('items',
        ['Alpha', 'Beta', 'Gamma', 'Delta', 'Item, with comma', 'Item "quoted"'],
        ArrayInput(new PropertySchema('item', '', TextInput))),
      new PropertySchema('value', ['Alpha'],
        ArrayInput(new PropertySchema('item', '', TextInput))),
      new PropertySchema('placeholder', 'Select items', TextInput),
      new PropertySchema('readOnly', false, BooleanInput),
      new PropertySchema('disabled', false, BooleanInput)],
    [new SignalSchema('onChange', 'value')],
    ListInputExample, 300);

const dateRangeInput =
  new ComponentSchema('DateRangeInput',
    [new PropertySchema('value', (() => {
        const today = Beam.Date.today();
        return new WebPortal.DateRange(
          new Beam.Date(today.year, today.month, 1), today);
      })(), DateRangeValueInput),
      new PropertySchema('label', 'Date range', TextInput),
      new PropertySchema('orientation',
        WebPortal.DateRangeInput.Orientation.VERTICAL,
        EnumInput(WebPortal.DateRangeInput.Orientation)),
      new PropertySchema('labelPosition', WebPortal.DateRangeInput.
        LabelPosition.ABOVE, EnumInput(WebPortal.DateRangeInput.LabelPosition)),
      new PropertySchema('boundsRequired', true, BooleanInput),
      new PropertySchema('readOnly', false, BooleanInput),
      new PropertySchema('disabled', false, BooleanInput)],
    [new SignalSchema('onChange', 'value'),
      new SignalSchema('onValidationChange', 'validation')],
    DateRangeInputExample, 284);

const decimalInput =
  new ComponentSchema('DecimalInput',
    [new PropertySchema('value', 100, NumberInput),
      new PropertySchema('min', 0, NumberInput),
      new PropertySchema('max', 1000, NumberInput),
      new PropertySchema('decimalPlaces', 2, NumberInput),
      new PropertySchema('readOnly', false, BooleanInput),
      new PropertySchema('disabled', false, BooleanInput)],
    [new SignalSchema('onChange', 'value')],
    (props: any) => React.createElement(WebPortal.DecimalInput,
      {...props, style: {width: '100%', ...props.style}}));

const intervalInput =
  new ComponentSchema('IntervalInput',
    [new PropertySchema('value', new WebPortal.Interval(1,
        WebPortal.Interval.Unit.DAY), IntervalValueInput),
      new PropertySchema('readOnly', false, BooleanInput),
      new PropertySchema('disabled', false, BooleanInput),
      new PropertySchema('required', false, BooleanInput)],
    [new SignalSchema('onChange', 'value')],
    WebPortal.IntervalInput, 284);

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
    (props: any) => React.createElement(WebPortal.MoneyInput,
      {...props, style: {width: '100%', ...props.style}}));

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
    [new PropertySchema('variant',
        WebPortal.NavigationTab.Variant.ICON_LABEL,
        EnumInput(WebPortal.NavigationTab.Variant)),
      new PropertySchema('current', 'requests/you', TextInput)],
    [new SignalSchema('onNavigate', 'current')],
    (props: any) =>
      React.createElement(WebPortal.NavigationHeader, props,
        ...NAVIGATION_TABS));

const notificationsFilterModal =
  new ComponentSchema('NotificationsFilterModal',
    [new PropertySchema('isOpen', false, BooleanInput)],
    [new SignalSchema('onSubmit', ''),
      new SignalSchema('onClose', 'isOpen')],
    (props: any) => {
      if(!props.isOpen) {
        return React.createElement('div', null, 'Modal is closed.');
      }
      return React.createElement(WebPortal.NotificationsFilterModal, {
        filter: {
          query: '',
          categories: new Set<Nexus.Notification.Category>(),
          startDate: Beam.Date.today(),
          endDate: Beam.Date.today()
        },
        onSubmit: props.onSubmit,
        onClose: () => props.onClose(false)
      });
    });

const notificationItem =
  new ComponentSchema('NotificationItem',
    [new PropertySchema('description',
        'Your request to update risk controls for achen01 has been approved.',
        TextInput),
      new PropertySchema('timestamp', (() => {
        const d = new Date();
        d.setHours(d.getHours() - 2);
        return d;
      })(), DateInput),
      new PropertySchema('url', '#', TextInput),
      new PropertySchema('isUnread', true, BooleanInput),
      new PropertySchema('isSelected', false, BooleanInput),
      new PropertySchema('hideIndicator', false, BooleanInput)],
    [new SignalSchema('onSelect', 'isSelected')],
    WebPortal.NotificationItem, -1);

const notificationItemPlaceholder =
  new ComponentSchema('NotificationItemPlaceholder',
    [],
    [],
    WebPortal.NotificationItemPlaceholder, -1);

const SAMPLE_NOTIFICATIONS: Nexus.Notification[] = [
  new Nexus.Notification('1', Beam.DirectoryEntry.INVALID,
    'Your request to update risk controls for achen01 has been approved.', '',
    Nexus.Notification.Category.ACCOUNT_MODIFICATION,
    Beam.DateTime.fromDate((() => {
      const d = new Date(); d.setHours(d.getHours() - 2); return d;
    })()), false),
  new Nexus.Notification('2', Beam.DirectoryEntry.INVALID,
    'New entitlements request from jberrios01 requires your review.', '',
    Nexus.Notification.Category.ACCOUNT_MODIFICATION,
    Beam.DateTime.fromDate((() => {
      const d = new Date(); d.setDate(d.getDate() - 1); return d;
    })()), false),
  new Nexus.Notification('3', Beam.DirectoryEntry.INVALID,
    'Risk parameters for trodriguez have been updated.', '',
    Nexus.Notification.Category.REPORT,
    Beam.DateTime.fromDate((() => {
      const d = new Date(); d.setDate(d.getDate() - 3); return d;
    })()), true)
];

const SAMPLE_NOTIFICATIONS_ALL_READ: Nexus.Notification[] = [
  new Nexus.Notification('1', Beam.DirectoryEntry.INVALID,
    'Your request to update risk controls for achen01 has been approved.', '',
    Nexus.Notification.Category.ACCOUNT_MODIFICATION,
    Beam.DateTime.fromDate((() => {
      const d = new Date(); d.setHours(d.getHours() - 2); return d;
    })()), true),
  new Nexus.Notification('3', Beam.DirectoryEntry.INVALID,
    'Risk parameters for trodriguez have been updated.', '',
    Nexus.Notification.Category.REPORT,
    Beam.DateTime.fromDate((() => {
      const d = new Date(); d.setDate(d.getDate() - 3); return d;
    })()), true)
];

enum PopoverMode {
  HAS_UNREAD,
  NO_UNREAD,
  EMPTY
}

const POPOVER_NOTIFICATIONS: Record<PopoverMode, Nexus.Notification[]> = {
  [PopoverMode.HAS_UNREAD]: SAMPLE_NOTIFICATIONS,
  [PopoverMode.NO_UNREAD]: SAMPLE_NOTIFICATIONS_ALL_READ,
  [PopoverMode.EMPTY]: []
};

const notificationsPopover =
  new ComponentSchema('NotificationsPopover',
    [new PropertySchema('mode', PopoverMode.HAS_UNREAD,
        EnumInput(PopoverMode))],
    [new SignalSchema('onDismissAll', ''),
      new SignalSchema('onOpen', ''),
      new SignalSchema('onClose', '')],
    (props: any) => React.createElement('div', null,
      React.createElement('button',
        {popovertarget: 'catalog-notifications-popover'},
        'Toggle Popover'),
      React.createElement(WebPortal.NotificationsPopover, {
        id: 'catalog-notifications-popover',
        notifications: POPOVER_NOTIFICATIONS[props.mode as PopoverMode],
        onDismissAll: props.onDismissAll,
        onOpen: props.onOpen,
        onClose: props.onClose
      })));

const notificationsButton =
  new ComponentSchema('NotificationsButton',
    [new PropertySchema('isCurrent', false, BooleanInput),
      new PropertySchema('hasUnread', true, BooleanInput),
      new PropertySchema('isOpen', false, BooleanInput)],
    [new SignalSchema('onClick', '')],
    WebPortal.NotificationsButton);

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
    WebPortal.Pagination, 732);

const scopeInput =
  new ComponentSchema('ScopeInput',
    [new PropertySchema('value',
        new Nexus.Scope(Nexus.Ticker.parse('ABX.TSX')), ScopeValueInput),
      new PropertySchema('readOnly', false, BooleanInput),
      new PropertySchema('disabled', false, BooleanInput),
      new PropertySchema('lookupDelay', 300, NumberInput),
      new PropertySchema('failLookup', false, BooleanInput)],
    [new SignalSchema('onChange', 'value')],
    ScopeInputExample, 300);

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
    (props: any) => React.createElement(WebPortal.Select,
      {...props, style: {width: '100%'}},
      React.createElement('option', {value: 'Apple'}, 'Apple'),
      React.createElement('option', {value: 'Banana'}, 'Banana'),
      React.createElement('option', {value: 'Cherry'}, 'Cherry'),
      React.createElement('option', {value: 'Grape'}, 'Grape'),
      React.createElement('option', {value: 'Mango'}, 'Mango'),
      React.createElement('option', {value: 'Orange'}, 'Orange')));

const skeleton =
  new ComponentSchema('Skeleton',
    [],
    [],
    WebPortal.Skeleton);

const tickerInput =
  new ComponentSchema('TickerInput',
    [new PropertySchema('value', Nexus.Ticker.parse('ABX.TSX'),
        TickerValueInput),
      new PropertySchema('readOnly', false, BooleanInput),
      new PropertySchema('disabled', false, BooleanInput),
      new PropertySchema('lookupDelay', 300, NumberInput),
      new PropertySchema('failLookup', false, BooleanInput)],
    [new SignalSchema('onChange', 'value')],
    TickerInputExample, 300);

const segmentButton =
  new ComponentSchema('SegmentButton',
    [new PropertySchema('name', 'demo-group', TextInput),
      new PropertySchema('label', 'Entitlements', TextInput),
      new PropertySchema('badge', '12', TextInput),
      new PropertySchema('isChecked', false, BooleanInput),
      new PropertySchema('disabled', false, BooleanInput)],
    [new SignalSchema('onChange', '')],
    (props: any) => React.createElement(WebPortal.SegmentButton,
      {...props, style: {width: '100%', ...props.style}}));

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
        new Nexus.Currency(840), 'USD', 'US Dollar', '$')
    }));

const USD_CURRENCY = new Nexus.CurrencyDatabase.Entry(
  new Nexus.Currency(840), 'USD', 'US Dollar', '$');

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
      new PropertySchema('deltaValue', '$50,000', TextInput),
      new PropertySchema('deltaDirection',
        WebPortal.RequestsModel.Direction.POSITIVE,
        EnumInput(WebPortal.RequestsModel.Direction))],
    [],
    (props: any) => {
      const {deltaValue, deltaDirection, ...rest} = props;
      return React.createElement(WebPortal.RiskControlsChangeItem, {
        ...rest,
        delta: {value: deltaValue, direction: deltaDirection}
      });
    });

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
    requester: Beam.DirectoryEntry.makeAccount(101, 'bmartin02'),
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
          totalCount: 5,
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

const segmentedSpinner =
  new ComponentSchema('SegmentedSpinner',
    [new PropertySchema('color', '#000000', ColorInput),
      new PropertySchema('size', 16, NumberSliderInput)],
    [],
    WebPortal.SegmentedSpinner);

const CURRENCY_TOOLTIP_SAMPLE_RATES:
    WebPortal.CurrencyTooltip.ExchangeRate[] = [
  {code: 'AUD', rate: '0.88'},
  {code: 'EUR', rate: '1.51'},
  {code: 'GBP', rate: '1.76'},
  {code: 'USD', rate: '1.36'}
];

const currencyTooltip =
  new ComponentSchema('CurrencyTooltip',
    [new PropertySchema('accountCurrency', 'CAD', TextInput)],
    [],
    (props: any) => React.createElement(WebPortal.CurrencyTooltip, {
      ...props,
      exchangeRates: CURRENCY_TOOLTIP_SAMPLE_RATES
    }));

const metric =
  new ComponentSchema('Metric',
    [new PropertySchema('id', 'total-pnl', TextInput),
      new PropertySchema('label', 'Total P/L', TextInput),
      new PropertySchema('value', '$1,234.56', TextInput),
      new PropertySchema('unit', 'CAD', TextInput),
      new PropertySchema('loading', false, BooleanInput)],
    [],
    WebPortal.Metric);

const PNL_HEADER_SAMPLE_RATES: WebPortal.CurrencyTooltip.ExchangeRate[] = [
  {code: 'AUD', rate: '0.88'},
  {code: 'EUR', rate: '1.51'},
  {code: 'USD', rate: '1.36'}
];

const profitAndLossHeader =
  new ComponentSchema('ProfitAndLossHeader',
    [new PropertySchema('symbol', '$', TextInput),
      new PropertySchema('code', 'CAD', TextInput),
      new PropertySchema('totalPnl', '1,234.56', TextInput),
      new PropertySchema('totalFees', '45.00', TextInput),
      new PropertySchema('totalVolume', '12,500.00', TextInput),
      new PropertySchema('loading', false, BooleanInput)],
    [],
    (props: any) => React.createElement(WebPortal.ProfitAndLossHeader, {
      ...props,
      foreignCurrencies: PNL_HEADER_SAMPLE_RATES
    }), 800);

const PNL_TABLE_SAMPLE_TICKERS: WebPortal.ProfitAndLossTable.Ticker[] = [
  {ticker: new Nexus.Ticker('AAPL', new Nexus.Venue('NASDAQ')),
    volume: Nexus.Quantity.parse('1250'),
    fees: Nexus.Money.parse('12.50'),
    profitAndLoss: Nexus.Money.parse('345.67')},
  {ticker: new Nexus.Ticker('MSFT', new Nexus.Venue('NASDAQ')),
    volume: Nexus.Quantity.parse('800'),
    fees: Nexus.Money.parse('8.00'),
    profitAndLoss: Nexus.Money.parse('-123.45')},
  {ticker: new Nexus.Ticker('GOOG', new Nexus.Venue('NASDAQ')),
    volume: Nexus.Quantity.parse('500'),
    fees: Nexus.Money.parse('5.00'),
    profitAndLoss: Nexus.Money.parse('678.90')},
  {ticker: new Nexus.Ticker('AMZN', new Nexus.Venue('NASDAQ')),
    volume: Nexus.Quantity.parse('300'),
    fees: Nexus.Money.parse('3.00'),
    profitAndLoss: Nexus.Money.parse('-45.20')}
];

const profitAndLossTable =
  new ComponentSchema('ProfitAndLossTable',
    [new PropertySchema('symbol', '$', TextInput)],
    [],
    (props: any) => React.createElement(WebPortal.ProfitAndLossTable, {
      symbol: props.symbol,
      totalProfitAndLoss: Nexus.Money.parse('855.92'),
      totalVolume: Nexus.Quantity.parse('2850'),
      totalFees: Nexus.Money.parse('28.50'),
      tickers: PNL_TABLE_SAMPLE_TICKERS
    }), 600);

const profitAndLossItem =
  new ComponentSchema('ProfitAndLossItem',
    [new PropertySchema('symbol', '$', TextInput),
      new PropertySchema('code', 'USD', TextInput)],
    [],
    (props: any) => React.createElement(WebPortal.ProfitAndLossItem, {
      symbol: props.symbol,
      code: props.code,
      totalProfitAndLoss: Nexus.Money.parse('855.92'),
      totalVolume: Nexus.Quantity.parse('2850'),
      totalFees: Nexus.Money.parse('28.50'),
      tickers: PNL_TABLE_SAMPLE_TICKERS
    }), 800);

const profitAndLossItemPlaceholder =
  new ComponentSchema('ProfitAndLossItemPlaceholder',
    [],
    [],
    WebPortal.ProfitAndLossItemPlaceholder);

const reportStatusIndicator =
  new ComponentSchema('ReportStatusIndicator',
    [new PropertySchema('id', 'report-status', TextInput),
      new PropertySchema('status',
        WebPortal.ReportStatusIndicator.Status.NONE,
        EnumInput(WebPortal.ReportStatusIndicator.Status))],
    [],
    WebPortal.ReportStatusIndicator);

const sortableTableHeaderCell =
  new ComponentSchema('SortableTableHeaderCell',
    [new PropertySchema('children', 'Column', TextInput),
      new PropertySchema('sortOrder',
        WebPortal.SortableTableHeaderCell.SortOrder.NONE,
        EnumInput(WebPortal.SortableTableHeaderCell.SortOrder)),
      new PropertySchema('textAlign', 'start', TextInput),
      new PropertySchema('disabled', false, BooleanInput),
      new PropertySchema('style', {}, CSSInput)],
    [new SignalSchema('onSort', 'sortOrder')],
    (props: any) => React.createElement('table', {
        style: {borderCollapse: 'collapse', width: '100%',
          fontFamily: 'Roboto, system-ui, sans-serif', fontSize: '0.875rem'}},
      React.createElement('thead', null,
        React.createElement('tr', null,
          React.createElement(WebPortal.SortableTableHeaderCell, props)))),
    180);

const inputErrorMessage =
  new ComponentSchema('InputErrorMessage',
    [new PropertySchema('error', WebPortal.ValidationError.NONE,
        EnumInput(WebPortal.ValidationError)),
      new PropertySchema('label', 'Name', TextInput),
      new PropertySchema('value', 'email address', TextInput),
      new PropertySchema('start', 'Start date', TextInput),
      new PropertySchema('end', 'End date', TextInput),
      new PropertySchema('style', {}, CSSInput)],
    [],
    WebPortal.InputErrorMessage);

const inputGroup =
  new ComponentSchema('InputGroup',
    [new PropertySchema('inputType', InputGroupExample.InputType.INTEGER,
        EnumInput(InputGroupExample.InputType)),
      new PropertySchema('label', 'Quantity', TextInput),
      new PropertySchema('inputId', '', TextInput),
      new PropertySchema('error', WebPortal.ValidationError.NONE,
        EnumInput(WebPortal.ValidationError)),
      new PropertySchema(
        'errorDisplay', InputGroupExample.ErrorDisplay.AUTOMATIC,
        EnumInput(InputGroupExample.ErrorDisplay)),
      new PropertySchema('customError', '', TextInput),
      new PropertySchema('readOnly', false, BooleanInput),
      new PropertySchema('disabled', false, BooleanInput)],
    [new SignalSchema('onChange', ''), new SignalSchema('onValidate', '')],
    InputGroupExample, 284);

const dateFilter =
  new ComponentSchema('DateFilter',
    [new PropertySchema('value', new WebPortal.DateRange(null, null),
        DateRangeValueInput),
      new PropertySchema('label', '', TextInput),
      new PropertySchema('today', Beam.Date.today(), BeamDateInput),
      new PropertySchema('readOnly', false, BooleanInput),
      new PropertySchema('disabled', false, BooleanInput)],
    [new SignalSchema('onChange', 'value'),
      new SignalSchema('onValidationChange', 'validation')],
    WebPortal.DateFilter, 284);

const parametersDateRangeInput =
  new ComponentSchema('ParametersDateRangeInput',
    [new PropertySchema('value', (() => {
        const today = Beam.Date.today();
        return new WebPortal.DateRange(
          new Beam.Date(today.year, today.month, 1), today);
      })(), DateRangeValueInput),
      new PropertySchema('readOnly', false, BooleanInput),
      new PropertySchema('disabled', false, BooleanInput)],
    [new SignalSchema('onChange', 'value'),
      new SignalSchema('onValidationChange', 'validation')],
    WebPortal.ParametersDateRangeInput, 284);

const activityTableRow =
  new ComponentSchema('ActivityTableRow',
    [new PropertySchema('id', '42', TextInput),
      new PropertySchema('type', 'Profit and Loss', TextInput),
      new PropertySchema('parameters', ['Canada', 'Alpha Group', 'Month to Date'],
        ArrayInput(new PropertySchema('parameter', 'Value', TextInput))),
      new PropertySchema('status',
        WebPortal.ReportActivityStatusTag.Status.GENERATING,
        EnumInput(WebPortal.ReportActivityStatusTag.Status)),
      new PropertySchema('dateModified', Beam.Date.today(), BeamDateInput),
      new PropertySchema('selected', false, BooleanInput)],
    [new SignalSchema('onSelect', '', [
      {parameterName: 'id', propertyName: ''},
      {parameterName: 'selected', propertyName: 'selected'}])],
    ActivityTableRowExample, 796);

const activityTableRowPlaceholder =
  new ComponentSchema('ActivityTableRowPlaceholder', [], [],
    ActivityTableRowPlaceholderExample, 796);

const reportActivityStatusTag =
  new ComponentSchema('ReportActivityStatusTag',
    [new PropertySchema('status',
        WebPortal.ReportActivityStatusTag.Status.GENERATING,
        EnumInput(WebPortal.ReportActivityStatusTag.Status)),
      new PropertySchema('style', {}, CSSInput)],
    [],
    WebPortal.ReportActivityStatusTag);

const reportTable =
  new ComponentSchema('ReportTable',
    [new PropertySchema('reportCount', 3, NumberInput),
      new PropertySchema('loading', false, BooleanInput),
      new PropertySchema('selected', [],
        ArrayInput(new PropertySchema('id', '1', TextInput))),
      new PropertySchema('sortColumn', WebPortal.ReportTable.Column.TYPE,
        EnumInput(WebPortal.ReportTable.Column)),
      new PropertySchema('sortOrder',
        WebPortal.SortableTableHeaderCell.SortOrder.NONE,
        EnumInput(WebPortal.SortableTableHeaderCell.SortOrder))],
    [new SignalSchema('onSelectionChange', 'selected'),
      new SignalSchema('onSort', '', [
        {parameterName: 'column', propertyName: 'sortColumn'},
        {parameterName: 'order', propertyName: 'sortOrder'}]),
      new SignalSchema('onNavigate', '')],
    ReportTableExample, 696);

const reportTableRow =
  new ComponentSchema('ReportTableRow',
    [new PropertySchema('id', '42', TextInput),
      new PropertySchema('type', 'Profit and Loss', TextInput),
      new PropertySchema('parameters', ['Canada', 'Alpha Group', 'Month to Date'],
        ArrayInput(new PropertySchema('parameter', 'Value', TextInput))),
      new PropertySchema('url', '/reports/42', TextInput),
      new PropertySchema('dateCreated', Beam.Date.today(), BeamDateInput),
      new PropertySchema('selected', false, BooleanInput)],
    [new SignalSchema('onSelect', '', [
        {parameterName: 'id', propertyName: ''},
        {parameterName: 'selected', propertyName: 'selected'}]),
      new SignalSchema('onNavigate', '')],
    ReportTableRowExample, 696);

const reportTableRowPlaceholder =
  new ComponentSchema('ReportTableRowPlaceholder', [], [],
    ReportTableRowPlaceholderExample, 696);

const reportTypeSelect =
  new ComponentSchema('ReportTypeSelect',
    [new PropertySchema('reportTypes',
        ['Profit and Loss', 'Entitlement Activity', 'Trading Volume'],
        ArrayInput(new PropertySchema('reportType', 'New report', TextInput))),
      new PropertySchema('value', 'Profit and Loss', TextInput),
      new PropertySchema('readOnly', false, BooleanInput),
      new PropertySchema('disabled', false, BooleanInput),
      new PropertySchema('aria-label', 'Report type', TextInput),
      new PropertySchema('style', {}, CSSInput)],
    [new SignalSchema('onChange', 'value')],
    WebPortal.ReportTypeSelect);

const scheduledDate =
  new ComponentSchema('ScheduledDate',
    [new PropertySchema('date', (() => {
        const today = new Date();
        return new Date(today.getFullYear(), today.getMonth(),
          today.getDate() + 17);
      })(), DateInput),
      new PropertySchema('today', new Date(), DateInput),
      new PropertySchema('repeats', false, BooleanInput),
      new PropertySchema('style', {}, CSSInput)],
    [],
    (props: any) => React.createElement(WebPortal.ScheduledDate, {
      ...props,
      date: {
        value: `${String(props.date.getFullYear()).padStart(4, '0')}-` +
          `${String(props.date.getMonth() + 1).padStart(2, '0')}-` +
          String(props.date.getDate()).padStart(2, '0'),
        label: props.date.toLocaleDateString('en-US', {
          month: 'short', day: '2-digit', year: 'numeric'
        })
      }
    }));

const scheduledReportItem =
  new ComponentSchema('ScheduledReportItem',
    [new PropertySchema('id', '42', TextInput),
      new PropertySchema('type', 'Profit and Loss', TextInput),
      new PropertySchema('parameters', [
        {label: 'Account / Group', value: 'Alpha Group'},
        {label: 'Scope', value: 'Canada'},
        {label: 'Date Range', value: 'Month to Date'}
      ], ArrayInput(new PropertySchema('parameter',
        {label: 'Parameter', value: 'Value'}, ScheduledReportParameterInput))),
      new PropertySchema('repeats', false, BooleanInput),
      new PropertySchema('runDate', (() => {
        const today = new Date();
        return new Date(today.getFullYear(), today.getMonth(),
          today.getDate() + 17);
      })(), DateInput)],
    [new SignalSchema('onRun', ''), new SignalSchema('onDuplicate', ''),
      new SignalSchema('onDelete', ''), new SignalSchema('onNavigate', '')],
    ScheduledReportItemExample, 320);

const scheduledReportItemContextMenu =
  new ComponentSchema('ScheduledReportItemContextMenu',
    [new PropertySchema('lastSubmitted', '', (props: any) =>
        React.createElement('output', null, props.value)),
      new PropertySchema('style', {}, CSSInput)],
    [new SignalSchema('onSubmit', 'lastSubmitted')],
    ScheduledReportItemContextMenuExample);

const scheduledReportItemPlaceholder =
  new ComponentSchema('ScheduledReportItemPlaceholder',
    [new PropertySchema('style', {}, CSSInput)],
    [],
    WebPortal.ScheduledReportItemPlaceholder, 320);

const shareReportModal =
  new ComponentSchema('ShareReportModal',
    [new PropertySchema('selected', [Beam.DirectoryEntry.makeAccount(1, 'Alice')],
        ArrayInput(new PropertySchema('recipient',
          Beam.DirectoryEntry.makeAccount(1, 'Alice'),
          BeamDirectoryEntryInput))),
      new PropertySchema('lookupDelay', 300, NumberInput)],
    [new SignalSchema('onSubmit', 'selected'), new SignalSchema('onClose', '')],
    ShareReportModalExample);

export const componentSections = [
  new ComponentSection('UI Kit', [button, buttonLink, burgerButton, checkbox,
    contextMenu, dateInput, dateTimeInput, decimalInput, disclosure,
    durationInput, expandButton, filterChip, filterInput, hLine, iconButton,
    iconLabelButton, input, integerField, intervalInput, labeledCheckbox, link,
    listInput, modal, navigationHeader, navigationTab, pagination, radioButton,
    segmentButton, segmentedControl, segmentedSpinner, select, skeleton,
    sortableTableHeaderCell, timeOfDayInput]),
  new ComponentSection('App Kit', [accountGroupListInput, countrySelect,
    currencySelect, dateRangeInput, emptyMessage, errorMessage,
    inputErrorMessage, inputGroup, moneyInput, pageLayout, relativeDate,
    roleIcon, rolePanel, scopeInput, tickerInput]),
  new ComponentSection('Notifications', [notificationsFilterModal,
    notificationItem, notificationItemPlaceholder, notificationsButton,
    notificationsPopover]),
  new ComponentSection('Profit and Loss Page', [currencyTooltip, metric,
    profitAndLossHeader, profitAndLossItem, profitAndLossItemPlaceholder,
    profitAndLossTable, reportStatusIndicator]),
  new ComponentSection('Report Page', [activityTableRow,
    activityTableRowPlaceholder,
    dateFilter, parametersDateRangeInput,
    reportActivityStatusTag, reportTable, reportTableRow,
    reportTableRowPlaceholder, reportTypeSelect, scheduledDate,
    scheduledReportItem, scheduledReportItemContextMenu,
    scheduledReportItemPlaceholder, shareReportModal]),
  new ComponentSection('Requests Page', [accountLink, changeTable,
    complianceRuleStatusTag, diffBadge, entitlementsChangeItem,
    entitlementsStatusTag, requestActivityItem, requestCategoryTag,
    requestDetailPage, requestDirectoryPage, requestEffectiveDate,
    requestFilterModal, requestItem, requestItemPlaceholder, requestSortSelect,
    requestStateIndicator, riskControlsChangeItem])];
