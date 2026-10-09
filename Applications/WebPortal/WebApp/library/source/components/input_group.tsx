import { css, StyleSheet } from 'aphrodite/no-important';
import * as React from 'react';
import { ValidationError } from '../models';
import { InputErrorMessage } from './input_error_message';

interface Properties extends
    Omit<React.HTMLAttributes<HTMLDivElement>, 'children' | 'onChange'> {

  /** The input's label. */
  label: string;

  /** One input that forwards id, className, and accessibility attributes. */
  children: React.ReactElement;

  /** The input's validity. Omit when the input has no validation errors. */
  validation?: InputGroup.ValidationState;

  /** Replaces the default InputErrorMessage with custom error content.
   * Supply this for FORMAT and OUT_OF_RANGE errors to specify their labels.
   */
  errorMessage?: React.ReactNode;

  /** Requests validation on mount and after input or value changes. */
  onValidate?: () => void;
}

interface State {
  showError: boolean;
}

/** Combines an input with its label and responsive validation message. */
export class InputGroup extends React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.state = {showError: false};
    this.identifier = `input-group-${InputGroup.nextId++}`;
    this.mounted = false;
    this.validationPending = false;
  }

  public render(): JSX.Element {
    const {label, children, validation, errorMessage, onValidate, className,
      ...attributes} = this.props;
    const child = React.Children.only(children);
    const inputId = child.props.id || `${this.identifier}-input`;
    const errorId = `${this.identifier}-error`;
    const visible = validation?.valid === false &&
      (validation.showError ?? this.state.showError);
    const describedBy = new Set<string>(
      (child.props['aria-describedby'] || '').split(/\s+/).filter(Boolean));
    if(visible) {
      describedBy.add(errorId);
    }
    const invalid = visible || child.props['aria-invalid'];
    const input = React.cloneElement(child, {
      id: inputId,
      className: [child.props.className, visible && 'error'].
        filter(Boolean).join(' '),
      style: {boxSizing: 'border-box', width: '100%', ...child.props.style},
      'aria-invalid': invalid,
      'aria-describedby': Array.from(describedBy).join(' ') || undefined,
      onChange: this.onChange
    });
    const message = errorMessage ??
      <InputErrorMessage label={label} error={validation?.error}/>;
    return <div {...attributes}
        className={[css(STYLES.container), className].join(' ')}>
      <Content label={label} inputId={inputId} input={input}
        onInput={this.onInput} onBlur={this.onBlur}
        errorId={errorId} visible={visible} message={message}/>
    </div>;
  }

  public componentDidMount(): void {
    this.mounted = true;
    this.props.onValidate?.();
  }

  public componentWillUnmount(): void {
    this.mounted = false;
  }

  private requestValidation(): void {
    if(!this.validationPending) {
      this.validationPending = true;
      queueMicrotask(() => {
        this.validationPending = false;
        if(this.mounted) {
          this.setState({showError: false});
          this.props.onValidate?.();
        }
      });
    }
  }

  private onChange = (...args: unknown[]) => {
    const result = this.props.children.props.onChange?.(...args);
    this.requestValidation();
    return result;
  };

  private onInput = () => {
    this.requestValidation();
  };

  private onBlur = (event: React.FocusEvent<HTMLDivElement>) => {
    if(!event.currentTarget.contains(event.relatedTarget as Node)) {
      queueMicrotask(() => {
        if(this.mounted) {
          this.setState({showError: this.props.validation?.valid === false});
        }
      });
    }
  };

  private static nextId = 0;
  private identifier: string;
  private mounted: boolean;
  private validationPending: boolean;
}

export namespace InputGroup {

  /** The validity and optional explicit error visibility of an input. */
  export interface ValidationState {

    /** Whether the input value is valid. */
    valid: boolean;

    /** The reason validation failed. */
    error: ValidationError;

    /** Whether to show the error; omit for automatic input/blur behavior. */
    showError?: boolean;
  }
}

interface ContentProperties {
  label: string;
  inputId: string;
  input: React.ReactElement;
  onInput: React.FormEventHandler<HTMLDivElement>;
  onBlur: React.FocusEventHandler<HTMLDivElement>;
  errorId: string;
  visible: boolean;
  message: React.ReactNode;
}

function Content(props: ContentProperties): JSX.Element {
  return <div>
    <div className={css(STYLES.fields)}>
      <Label label={props.label} inputId={props.inputId}/>
      <div className={css(STYLES.input)}
          onInput={props.onInput} onBlur={props.onBlur}>
        {props.input}
      </div>
    </div>
    <Error id={props.errorId} visible={props.visible}>
      {props.message}
    </Error>
  </div>;
}

function Label(props: {label: string; inputId: string}): JSX.Element {
  return <div className={css(STYLES.labelWrapper)}>
    <label htmlFor={props.inputId} className={css(STYLES.label)}>
      {props.label}
    </label>
  </div>;
}

interface ErrorProperties {
  id: string;
  visible: boolean;
  children: React.ReactNode;
}

interface ErrorState {
  height: number;
}

class Error extends React.Component<ErrorProperties, ErrorState> {
  constructor(props: ErrorProperties) {
    super(props);
    this.state = {height: 0};
    this.content = props.children;
    this.element = React.createRef();
    this.observer = null;
    this.frame = null;
  }

  public render(): JSX.Element {
    if(this.props.visible) {
      this.content = this.props.children;
    }
    const height = (() => {
      if(this.props.visible) {
        return this.state.height;
      }
      return 0;
    })();
    const attributes = (() => {
      if(!this.props.visible) {
        return {inert: ''};
      }
      return {};
    })();
    return <div {...attributes} id={this.props.id}
        aria-hidden={!this.props.visible} className={css(STYLES.error)}
        style={{maxHeight: height}}>
      <div ref={this.element} className={css(STYLES.errorContent)}>
        {this.content}
      </div>
    </div>;
  }

  public componentDidMount(): void {
    this.observer = new ResizeObserver(this.onResize);
    this.observer.observe(this.element.current);
    this.scheduleMeasure();
  }

  public componentDidUpdate(): void {
    this.scheduleMeasure();
  }

  public componentWillUnmount(): void {
    this.observer.disconnect();
    window.cancelAnimationFrame(this.frame);
  }

  private scheduleMeasure(): void {
    if(this.frame === null) {
      this.frame = window.requestAnimationFrame(this.onMeasure);
    }
  }

  private onResize = () => {
    this.scheduleMeasure();
  };

  private onMeasure = () => {
    this.frame = null;
    const height = this.element.current.getBoundingClientRect().height;
    if(height !== this.state.height) {
      this.setState({height});
    }
  };

  private content: React.ReactNode;
  private element: React.RefObject<HTMLDivElement>;
  private observer: ResizeObserver;
  private frame: number;
}

const STYLES = StyleSheet.create({
  container: {
    containerType: 'inline-size',
    boxSizing: 'border-box',
    width: '100%',
    backgroundColor: '#FFFFFF',
    color: '#333333',
    fontFamily: 'Roboto, system-ui, sans-serif',
    fontWeight: 400,
    fontSize: '0.875rem'
  },
  fields: {
    display: 'grid',
    alignItems: 'flex-start',
    '@container (width < 384px)': {gridTemplateColumns: 'minmax(0, 1fr)'},
    '@container (min-width: 384px)': {
      gridTemplateColumns: '130px 246px',
      columnGap: '8px'
    }
  },
  labelWrapper: {
    '@container (width < 384px)': {marginBottom: '12px'},
    '@container (min-width: 384px)': {
      display: 'flex',
      alignItems: 'center',
      height: '34px'
    }
  },
  label: {
    '@container (width < 384px)': {paddingInlineStart: '10px'},
    '@container (min-width: 384px)': {paddingInlineStart: 0}
  },
  input: {minWidth: 0},
  error: {
    overflow: 'hidden',
    transition: 'max-height 200ms ease-in-out'
  },
  errorContent: {
    paddingTop: '4px',
    '@container (min-width: 384px)': {paddingLeft: '138px'}
  }
});
