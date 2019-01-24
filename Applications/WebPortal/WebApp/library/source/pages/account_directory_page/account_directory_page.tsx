import { css, StyleSheet } from 'aphrodite';
import * as React from 'react';
import { DisplaySize } from '../../display_size';
import { AccountDirectoryModel, FilterBar } from '.';

interface Properties {

  /** Determines the layout used to display the page. */
  displaySize: DisplaySize;

  /** Called when the user wants to make a new group. */
  onNewGroupClick?: () => {};

  /** Called when the user wants to make a new account. */
  onNewAccountClick?: () => {};

  /** Model that contains imformation about the accounts. */
  model: AccountDirectoryModel;
}

interface State {
  filter: string;
}

/** Displays an directory of accounts. */
export class AccountDirectoryPage extends React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.state = {
      filter: ''
    };
    this.onChange = this.onChange.bind(this);
  }

  public render(): JSX.Element {
    const contentWidth = (() => {
      switch(this.props.displaySize) {
        case DisplaySize.SMALL:
          return AccountDirectoryPage.STYLE.contentSmall;
        case DisplaySize.MEDIUM:
          return AccountDirectoryPage.STYLE.contentMedium;
        case DisplaySize.LARGE:
          return AccountDirectoryPage.STYLE.contentLarge;
      }
    })();
    const headerBoxStyle = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return AccountDirectoryPage.STYLE.verticalHeaderBox;
      } else {
        return AccountDirectoryPage.STYLE.horizontalHeaderBox;
      }
    })();
    const buttonBoxStyle = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return AccountDirectoryPage.STYLE.buttonBoxSmall;
      } else {
        return AccountDirectoryPage.STYLE.buttonBox;
      }
    })();
    const buttonStyle = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return AccountDirectoryPage.DYNAMIC_STYLE.buttonSmall;
      } else {
        return AccountDirectoryPage.DYNAMIC_STYLE.button;
      }
    })();
    const cards = (() => {
      return <div/>;
    })();
    return (
      <div style={AccountDirectoryPage.STYLE.page}>
        <div style={contentWidth}>
          <div id={'header'} style={headerBoxStyle}>
            <FilterBar value={this.state.filter} onChange={this.onChange}/>
            <div style={buttonBoxStyle}>
              <button className={css(buttonStyle)}
                  onClick={this.props.onNewAccountClick}>
                New Account
              </button>
              <div style={AccountDirectoryPage.STYLE.buttonSpacing}/>
              <button onClick={this.props.onNewGroupClick}
                  className={css(buttonStyle)}>
                New Group
              </button>
            </div>
          </div>
          <div id={'group_cards'}>
            {cards}
          </div>
        </div>
      </div>);
  }

  private onChange(newFilter: string) {
    this.setState({filter: newFilter});
  }

  private static readonly STYLE = {
    page: {
      boxSizing: 'border-box' as 'border-box',
      height: '100%',
      width: '100%',
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      overflowY: 'auto' as 'auto',
      justifyContent: 'center',
      padding: '18px'
    },
    contentSmall: {
      flexGrow: 1,
      maxWidth: '424px'
    },
    contentMedium: {
      width: '732px'
    },
    contentLarge: {
      width: '1000px'
    },
    verticalHeaderBox: {
      display: 'flex' as 'flex',
      flexDirection: 'column-reverse' as 'column-reverse',
      flexWrap: 'nowrap' as 'nowrap',
      height: '86px',
      justifyContent: 'space-between' as 'space-between'
    },
    horizontalHeaderBox: {
      boxSizing: 'border-box' as 'border-box',
      width: '100%',
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      flexWrap: 'nowrap' as 'nowrap'
    },
    buttonBoxSmall: {
      width: '100%',
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      flexWrap: 'nowrap' as 'nowrap',
    },
    buttonBox: {
      boxSizing: 'border-box' as 'border-box',
      flexBasis: '316px',
      flexGrow: 0,
      flexShrink: 0,
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      flexWrap: 'nowrap' as 'nowrap',
      justifyContent: 'flex-end' as 'flex-end'
    },
    buttonSpacing: {
      width: '18px'
    },
    filler: {
      flexGrow: 1
    }
  };

  private static DYNAMIC_STYLE = StyleSheet.create({
    button: {
      boxSizing: 'border-box' as 'border-box',
      width: '140px',
      flexBasis: '140px',
      flexGrow: 0,
      flexShrink: 0,
      height: '34px',
      backgroundColor: '#684BC7',
      font: '400 14px Roboto',
      color: '#FFFFFF',
      border: 'none',
      outline: 0,
      borderRadius: 1,
      cursor: 'pointer' as 'pointer',
      ':active': {
        backgroundColor: '#4B23A0'
      },
      ':focus' : {
        backgroundColor: '#4B23A0'
      },
      ':hover' : {
        backgroundColor: '#4B23A0'
      },
      ':disabled': {
        backgroundColor: '#F8F8F8',
        color: '#8C8C8C',
        cursor: 'default' as 'default'
      }
    },
    buttonSmall: {
      boxSizing: 'border-box' as 'border-box',
      width: '140px',
      flexBasis: '140px',
      flexGrow: 1,
      height: '34px',
      backgroundColor: '#684BC7',
      font: '400 14px Roboto',
      color: '#FFFFFF',
      border: 'none',
      outline: 0,
      borderRadius: 1,
      cursor: 'pointer' as 'pointer',
      ':active': {
        backgroundColor: '#4B23A0'
      },
      ':focus' : {
        backgroundColor: '#4B23A0'
      },
      ':hover' : {
        backgroundColor: '#4B23A0'
      },
      ':disabled': {
        backgroundColor: '#F8F8F8',
        color: '#8C8C8C',
        cursor: 'default' as 'default'
      }
  });
}
