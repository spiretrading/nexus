import { css, StyleSheet } from 'aphrodite/no-important';
import * as React from 'react';
import { QueryModel } from '../models';
import { EditListModal } from './edit_list_modal';
import { Input } from './input';

interface Properties<T> extends
    Omit<React.InputHTMLAttributes<HTMLInputElement>,
      'value' | 'defaultValue' | 'onChange'> {

  /** The selected items displayed in the input. */
  value: readonly T[];

  /** Resolves queries for suggestions and CSV import in the editor. */
  model: QueryModel<T>;

  /** Returns the label displayed for an item. */
  getLabel: (item: T) => string;

  /** Compares items for selection and deduplication. */
  isEqual?: (first: T, second: T) => boolean;

  /** The title of the editor. */
  title: string;

  /** The heading above the selected items in the editor. */
  listHeading: string;

  /** The editor component. Defaults to EditListModal. */
  editListModal?:
    React.JSXElementConstructor<React.ComponentProps<typeof EditListModal<T>>>;

  /** Called when the user submits a new selection.
   * @param value - The updated selection.
   */
  onChange?: (value: T[]) => void;
}

interface State {
  isOpen: boolean;
}

/** Displays a comma-separated selection and opens a list editor on focus. */
export class ListInput<T> extends React.Component<Properties<T>, State> {
  constructor(props: Properties<T>) {
    super(props);
    this.state = {isOpen: false};
    this.input = React.createRef<HTMLInputElement>();
    this.closing = false;
  }

  public componentDidUpdate(previous: Properties<T>): void {
    if(!previous.disabled && this.props.disabled && this.state.isOpen) {
      this.onClose();
    }
  }

  public render(): JSX.Element {
    const {value, model, getLabel, isEqual, title, listHeading, editListModal,
      onChange, readOnly, className, style, onFocus, onClick, ...rest} =
        this.props;
    const Modal = editListModal ?? EditListModal<T>;
    return (
      <>
        <Input {...rest} ref={this.input} type='text' readOnly
          value={value.map(getLabel).join(', ')}
          aria-haspopup='dialog' aria-expanded={this.state.isOpen}
          className={[css(STYLES.input, !readOnly && STYLES.interactive),
            className].join(' ')}
          style={{width: '100%', ...style}}
          onFocus={this.onFocus} onClick={this.onClick}/>
        {this.state.isOpen &&
          <Modal title={title} listHeading={listHeading}
            model={model} selected={value} getLabel={getLabel} isEqual={isEqual}
            selectionMode={EditListModal.SelectionMode.MULTIPLE}
            readOnly={readOnly} onSubmit={this.onSubmit}
            onClose={this.onClose}/>}
      </>);
  }

  private open(): void {
    if(!this.props.disabled && !this.closing && !this.state.isOpen) {
      this.setState({isOpen: true});
    }
  }

  private onFocus = (event: React.FocusEvent<HTMLInputElement>) => {
    this.props.onFocus?.(event);
    if(!event.defaultPrevented) {
      this.open();
    }
  };

  private onClick = (event: React.MouseEvent<HTMLInputElement>) => {
    this.props.onClick?.(event);
    if(!event.defaultPrevented) {
      this.open();
    }
  };

  private onClose = () => {
    this.closing = true;
    this.setState({isOpen: false}, () => {
      this.input.current?.blur();
      this.closing = false;
    });
  };

  private onSubmit = (value: T[]) => {
    this.onClose();
    this.props.onChange?.(value);
  };

  private input: React.RefObject<HTMLInputElement>;
  private closing: boolean;
}

const STYLES = StyleSheet.create({
  input: {
    textOverflow: 'ellipsis',
    ':enabled': {
      cursor: 'pointer'
    }
  },
  interactive: {
    borderColor: '#C8C8C8 !important',
    ':enabled:hover': {
      borderColor: '#684BC7 !important'
    },
    ':enabled:focus': {
      borderColor: '#684BC7 !important'
    }
  }
});
