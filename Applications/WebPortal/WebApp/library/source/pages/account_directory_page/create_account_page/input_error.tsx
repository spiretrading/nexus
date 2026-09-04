import * as React from 'react';
import { DisplaySize } from '../../..';
import { CreateAccountModel } from './create_account_model';
import { InputValidation } from './input_validation';

interface Properties {

  /** The size of the element to display. */
  displaySize: DisplaySize;

  /** The name of the input the error refers to. */
  label: string;

  /** The validation state of the input. */
  validation: InputValidation;
}

interface State {
  height: number;
}

/** Displays the validation error belonging to a single input. */
export class InputError extends React.Component<Properties, State> {
  public constructor(props: Properties) {
    super(props);
    this.state = {height: 0};
    this._content = React.createRef<HTMLDivElement>();
  }

  public render(): JSX.Element {
    const maxHeight = (() => {
      if(this.props.validation.valid || !this.props.validation.showError) {
        return 0;
      }
      return this.state.height;
    })();
    const indent = (() => {
      switch(this.props.displaySize) {
        case DisplaySize.SMALL:
          return null;
        case DisplaySize.MEDIUM:
          return <div style={InputError.STYLE.mediumIndent}/>;
        case DisplaySize.LARGE:
          return <div style={InputError.STYLE.largeIndent}/>;
      }
    })();
    return (
      <div style={{...InputError.STYLE.container, maxHeight: maxHeight}}>
        <div ref={this._content} style={InputError.STYLE.content}>
          <div style={InputError.STYLE.topPadding}/>
          <div style={InputError.STYLE.row}>
            {indent}
            <span style={InputError.STYLE.message}>{this.getMessage()}</span>
          </div>
        </div>
      </div>);
  }

  public componentDidMount(): void {
    this.observe();
  }

  public componentDidUpdate(): void {
    this.observe();
  }

  public componentWillUnmount(): void {
    this._observer?.disconnect();
  }

  private observe(): void {
    if(!this._content.current || this._observer) {
      return;
    }
    this._observer = new ResizeObserver(entries => {
      for(const entry of entries) {
        this.setState({height: entry.contentRect.height});
      }
    });
    this._observer.observe(this._content.current);
  }

  private getMessage(): string {
    switch(this.props.validation.error) {
      case CreateAccountModel.ValidationError.REQUIRED:
        return `${this.props.label} cannot be empty`;
      case CreateAccountModel.ValidationError.DUPLICATE:
        return `${this.props.label} is already in use`;
      case CreateAccountModel.ValidationError.FORMAT:
        return `${this.props.label} contains invalid characters`;
      default:
        return '';
    }
  }

  private static readonly STYLE: Record<string, React.CSSProperties> = {
    container: {
      overflow: 'hidden',
      transition: 'max-height 200ms ease-in-out'
    },
    content: {
      display: 'flex',
      flexDirection: 'column'
    },
    topPadding: {
      height: '4px',
      flexShrink: 0
    },
    row: {
      display: 'flex',
      flexDirection: 'row'
    },
    mediumIndent: {
      width: '138px',
      minWidth: '138px',
      flexShrink: 0
    },
    largeIndent: {
      width: '170px',
      minWidth: '170px',
      flexShrink: 0
    },
    message: {
      flexGrow: 1,
      fontSize: '0.875rem',
      color: '#E63F44'
    }
  };
  private _content: React.RefObject<HTMLDivElement>;
  private _observer: ResizeObserver;
}
