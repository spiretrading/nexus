import { css, StyleSheet } from 'aphrodite/no-important';
import * as React from 'react';

interface Properties {

  /** The id used by an invoking element to target the menu. */
  id?: string;

  /** The ordered list of menu entries. */
  items: ContextMenu.Entry[];

  /** The element that opens the menu. */
  invoker?: React.RefObject<HTMLElement>;

  /** The class used to position the menu. */
  className?: string;

  /** The style used to position the menu. */
  style?: React.CSSProperties;

  /** Called when a menu command is submitted.
   * @param value - The value of the menu command.
   */
  onSubmit?: (value: any) => void;
}

interface State {
  currentIndex: number;
  checkedIndexes: {[group: string]: number};
}

/** Displays a contextual menu of commands. */
export class ContextMenu extends React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.state = {
      currentIndex: -1,
      checkedIndexes: getCheckedIndexes(props.items)
    };
    this.menu = React.createRef();
  }

  public componentDidMount(): void {
    this.menu.current.addEventListener('toggle', this.onToggle);
    this.resetTabIndexes();
  }

  public componentDidUpdate(previousProperties: Properties): void {
    if(previousProperties.items !== this.props.items) {
      this.setState({checkedIndexes: getCheckedIndexes(this.props.items)});
      this.resetTabIndexes();
    }
  }

  public componentWillUnmount(): void {
    this.menu.current?.removeEventListener('toggle', this.onToggle);
  }

  public render(): JSX.Element {
    return (
      <div ref={this.menu} id={this.props.id} tabIndex={-1}
          {...{popover: 'auto'} as any}
          className={[css(STYLES.contextMenu), this.props.className].join(' ')}
          style={this.props.style} onKeyDown={this.onKeyDown}
          onFocus={this.onFocus}>
        <menu role='menu' className={css(STYLES.menu)}
            onPointerMove={this.onPointerMove}>
          {this.props.items.map((item, index) =>
            <li key={index} role='presentation'
                onClick={(event) => this.onClick(event, index)}>
              {this.renderBody(item, index)}
            </li>)}
        </menu>
      </div>);
  }

  private menu: React.RefObject<HTMLDivElement>;

  private get focusableItems(): HTMLElement[] {
    if(!this.menu.current) {
      return [];
    }
    return Array.from(this.menu.current.querySelectorAll<HTMLElement>(
      '[role="menuitem"], [role="menuitemradio"]'));
  }

  private renderBody = (item: ContextMenu.Entry, index: number) => {
    if(item.body.type !== ContextMenu.ItemRadio) {
      return item.body;
    }
    return React.cloneElement(item.body, {
      checked: this.state.checkedIndexes[item.body.props.group] === index
    });
  };

  private getItem = (target: HTMLElement) => {
    return target.closest<HTMLElement>(
      '[role="menuitem"], [role="menuitemradio"]');
  };

  private resetTabIndexes = () => {
    for(const item of this.focusableItems) {
      item.tabIndex = -1;
    }
  };

  private moveFocus = (index: number) => {
    const items = this.focusableItems;
    if(index < 0 || index >= items.length) {
      return;
    }
    this.resetTabIndexes();
    items[index].tabIndex = 0;
    items[index].focus();
  };

  private onKeyDown = (event: React.KeyboardEvent<HTMLDivElement>) => {
    const items = this.focusableItems;
    if(items.length === 0) {
      return;
    }
    const index = (() => {
      if(event.key === 'Home') {
        return 0;
      } else if(event.key === 'End') {
        return items.length - 1;
      } else if(event.key === 'ArrowUp') {
        if(this.state.currentIndex === -1) {
          return items.length - 1;
        }
        return (this.state.currentIndex - 1 + items.length) % items.length;
      } else if(event.key === 'ArrowDown') {
        return (this.state.currentIndex + 1) % items.length;
      }
      return -1;
    })();
    if(index === -1) {
      return;
    }
    event.preventDefault();
    this.moveFocus(index);
  };

  private onFocus = (event: React.FocusEvent<HTMLDivElement>) => {
    const item = this.getItem(event.target as HTMLElement);
    if(!item) {
      return;
    }
    this.setState({currentIndex: this.focusableItems.indexOf(item)});
  };

  private onPointerMove = (event: React.PointerEvent<HTMLElement>) => {
    const item = this.getItem(event.target as HTMLElement);
    if(!item || item === document.activeElement) {
      return;
    }
    this.moveFocus(this.focusableItems.indexOf(item));
  };

  private onClick = (event: React.MouseEvent<HTMLLIElement>,
      index: number) => {
    if(!this.getItem(event.target as HTMLElement)) {
      return;
    }
    const item = this.props.items[index];
    if(item.body.type === ContextMenu.ItemRadio) {
      const group = item.body.props.group;
      this.setState((state: State) => ({
        checkedIndexes: {...state.checkedIndexes, [group]: index}
      }));
    }
    this.menu.current.hidePopover();
    this.props.onSubmit?.(item.value);
  };

  private onToggle = (event: Event) => {
    if((event as any).newState === 'open') {
      this.menu.current.focus();
    } else {
      this.setState({currentIndex: -1});
      this.resetTabIndexes();
      this.props.invoker?.current?.focus();
    }
  };
}

interface ItemProperties {

  /** The label for the item. */
  label: string;

  /** Called when the item is clicked. */
  onClick?: () => void;
}

interface ItemRadioProperties {

  /** The label for the item. */
  label: string;

  /** The name of the radio group to which this item belongs. */
  group: string;

  /** Whether the radio button is checked. Defaults to false. */
  checked?: boolean;

  /** Called when the item is clicked. */
  onClick?: () => void;
}

export namespace ContextMenu {

  /** An entry within a ContextMenu. */
  export interface Entry {

    /** The component used to render the item. */
    body: JSX.Element;

    /** The value submitted when the item is chosen. */
    value?: any;
  }

  /** A command rendered within a ContextMenu. */
  export class Item extends React.Component<ItemProperties> {
    public render(): JSX.Element {
      return (
        <button type='button' role='menuitem' tabIndex={-1}
            className={css(STYLES.item)} onClick={this.props.onClick}>
          {this.props.label}
        </button>);
    }
  }

  /** An option belonging to a mutually exclusive set of options rendered
   *  within a ContextMenu.
   */
  export class ItemRadio extends React.Component<ItemRadioProperties> {
    public render(): JSX.Element {
      const checked = this.props.checked ?? false;
      return (
        <button type='button' role='menuitemradio' aria-checked={checked}
            tabIndex={-1} className={css(STYLES.itemRadio)}
            onClick={this.props.onClick}>
          <img aria-hidden='true' width='20' height='20'
            src='resources/components/checkmark.svg'
            className={css(STYLES.check, checked && STYLES.checkedCheck)}/>
          <div className={css(STYLES.label)}>{this.props.label}</div>
        </button>);
    }
  }

  /** A visual separator between related groups of commands. */
  export function Separator(): JSX.Element {
    return (
      <div role='separator' className={css(STYLES.separator)}>
        <div className={css(STYLES.horizontalRule)}/>
      </div>);
  }
}

function getCheckedIndexes(items: ContextMenu.Entry[]) {
  const checkedIndexes = {} as {[group: string]: number};
  items.forEach((item, index) => {
    if(item.body.type === ContextMenu.ItemRadio && item.body.props.checked) {
      checkedIndexes[item.body.props.group] = index;
    }
  });
  return checkedIndexes;
}

const STYLES = StyleSheet.create({
  contextMenu: {
    boxSizing: 'border-box',
    display: 'flex',
    flexDirection: 'column',
    minWidth: '192px',
    maxHeight: 'calc(100dvh - 80px)',
    backgroundColor: '#FFFFFF',
    border: 'none',
    outline: 'none',
    borderRadius: '1px',
    boxShadow: '0 0 6px rgb(0 0 0 / 0.40)',
    padding: '8px 0',
    overflow: 'hidden',
    fontSize: '0.875rem',
    color: '#333333',
    fontFamily: '"Roboto", system-ui, sans-serif',
    opacity: 0,
    transitionProperty: 'opacity, display, overlay',
    transitionDuration: '200ms',
    transitionTimingFunction: 'ease-out',
    transitionBehavior: 'allow-discrete',
    ':popover-open': {
      opacity: 1
    },
    '@starting-style': {
      opacity: 0
    }
  },
  menu: {
    listStyle: 'none',
    margin: 0,
    padding: 0,
    minHeight: 0,
    overflowY: 'auto',
    overscrollBehavior: 'contain',
    containerType: 'inline-size',
    ':has([role="menuitemradio"]) [role="menuitem"]': {
      paddingInlineStart: '36px'
    }
  },
  item: {
    boxSizing: 'border-box',
    display: 'flex',
    alignItems: 'center',
    width: '100%',
    height: '34px',
    backgroundColor: '#FFFFFF',
    border: 'none',
    outline: 'none',
    borderRadius: 0,
    paddingInline: '18px',
    fontSize: '0.875rem',
    color: '#333333',
    fontFamily: '"Roboto", system-ui, sans-serif',
    cursor: 'pointer',
    ':focus': {
      backgroundColor: '#F8F8F8'
    }
  },
  itemRadio: {
    boxSizing: 'border-box',
    display: 'flex',
    alignItems: 'center',
    width: '100%',
    height: '34px',
    backgroundColor: '#FFFFFF',
    border: 'none',
    outline: 'none',
    borderRadius: 0,
    padding: '0 18px 0 8px',
    fontSize: '0.875rem',
    color: '#333333',
    fontFamily: '"Roboto", system-ui, sans-serif',
    cursor: 'pointer',
    ':focus': {
      backgroundColor: '#F8F8F8'
    }
  },
  check: {
    flexGrow: 0,
    flexShrink: 0,
    width: '20px',
    height: '20px',
    visibility: 'hidden'
  },
  checkedCheck: {
    visibility: 'visible'
  },
  label: {
    flexGrow: 1,
    flexShrink: 1,
    marginInlineStart: '8px',
    textAlign: 'start'
  },
  separator: {
    backgroundColor: '#FFFFFF',
    paddingBlock: '4px'
  },
  horizontalRule: {
    height: '1px',
    backgroundColor: '#E6E6E6'
  }
});
