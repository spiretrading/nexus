import { css, StyleSheet } from 'aphrodite/no-important';
import * as React from 'react';
import { Transition } from 'react-transition-group';
import { HBoxLayout, Padding, VBoxLayout } from '..';

interface Properties {

  /** Time in Milliseconds before displaying the loading animation. */
  delay?: number;
}

interface State {

  /** Time in Milliseconds before displaying the loading animation. */
  showAnimation: boolean;
}

/** Displays the main dashboard. */
export class LoadingPage extends React.Component<Properties, State> {
  public static readonly defaultProps = {
    delay: 0
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
        <Padding size={LoadingPage.TOP_PADDING} />
        <HBoxLayout>
          <Padding />

          <Transition in={this.state.showAnimation} 
          timeout={this.props.delay} mountOnEnter={true}>
            {(state) => (
              <div style={{ ...(LoadingPage.DELAY as any)[state] }}>
                <img src='resources/loading/pre-loader.gif'
                  style={LoadingPage.STYLE.containerStyle} />
              </div>)}
          </Transition>
          <Padding />
        </HBoxLayout>
        <Padding size={LoadingPage.BOTTOM_PADDING} />
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
