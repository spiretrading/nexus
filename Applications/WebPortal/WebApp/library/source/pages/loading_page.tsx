import { HBoxLayout, Padding, VBoxLayout } from 'dali';
import * as React from 'react';
import { Transition } from 'react-transition-group';

interface Properties {

  /** Amount of time in milliseconds before displaying the loading animation. */
  delay?: number;
}

interface State {
  showAnimation: boolean;
}

/** Displays a loading page. */
export class LoadingPage extends React.Component<Properties, State> {
  public static readonly defaultProps = {
    delay: 2000
  }

  constructor(props: Properties) {
    super(props);
    this.state = {
      showAnimation: false
    };
  }

  public render(): JSX.Element {
    return (
      <VBoxLayout width='100%' height='100%'>
        <Padding size={LoadingPage.TOP_PADDING}/>
        <HBoxLayout>
          <Padding/>
          <Transition in={this.state.showAnimation} timeout={this.props.delay}>
            {(state) => (
              <div style={{ ...(LoadingPage.DELAY as any)[state] }}>
                <img src='resources/loading/pre-loader.gif'
                  style={LoadingPage.STYLE.containerStyle}/>
              </div>)}
          </Transition>
          <Padding/>
        </HBoxLayout>
        <Padding size={LoadingPage.BOTTOM_PADDING}/>
      </VBoxLayout>);
  }

  public componentDidMount(): void {
    this.setState({showAnimation: true});
  }

  private static DELAY = {
    entering: {
      opacity: 0
    },
    entered: {
      opacity: 1
    }
  };
  private static readonly STYLE = {
    containerStyle: {
      height: '30px',
      width: '30px'
    }
  };
  private static readonly TOP_PADDING = '150px';
  private static readonly BOTTOM_PADDING = '40px';
}

/** Helper class for keeping track of a page's loading state. */
export class LoadingState {

  /**
   * Constructs a LoadingState in a given state with a given message. By default
   * the state is LOADING with an empty message.
   */
  constructor(state?: LoadingState.State, message?: string) {
    if(state) {
      this._state = state;
    } else {
      this._state = LoadingState.State.LOADING;
    }
    if(message) {
      this._message = message;
    } else {
      this._message = '';
    }
  }

  /** Returns the current state. */
  public get state(): LoadingState.State {
    return this._state;
  }

  /** Returns the current message. */
  public get message(): string {
    if(this._state === LoadingState.State.ERROR) {
      return this._message;
    }
    return '';
  }

  /** Returns true iff the state is LOADING. */
  public isLoading(): boolean {
    return this.state === LoadingState.State.LOADING;
  }

  /** Updates the state to indicate success. */
  public succeed(): void {
    this._state = LoadingState.State.LOADED;
  }

  /** Updates the state to indicate failure. */
  public fail(message?: string): void {
    this._state = LoadingState.State.ERROR;
    if(message) {
      this._message = message;
    }
  }

  private _state: LoadingState.State;
  private _message: string;
}

export namespace LoadingState {

  /** Enumerates a page's loading states. */
  export enum State {

    /** The initial state indicating the page is loading. */
    LOADING,

    /** A terminal state for when the page successfully loaded. */
    LOADED,

    /** A terminal state for when the page failed to load. */
    ERROR
  }
}
