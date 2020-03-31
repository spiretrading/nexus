import * as Nexus from 'nexus';
import * as React from 'react';
import { Button, DisplaySize, PageWrapper } from '../../..';
import { SubmissionBox } from '..';
import { RiskParametersView } from '.';
import { HLine } from '../../../components';

interface Properties {

  /** The type of display to render on. */
  displaySize: DisplaySize;

  /** The parameters to display. */
  parameters: Nexus.RiskParameters;

  /** Used to lookup currency names and symbols. */
  currencyDatabase: Nexus.CurrencyDatabase;

  roles: Nexus.AccountRoles;

  onSubmit?: () => void;

  status?: string;

  isError?: boolean;
}

interface State {
  
}

export class RiskPage extends React.Component<Properties> {
  render() {
    const containerStyle= (() => {
      switch(this.props.displaySize) {
        case DisplaySize.SMALL:
          return RiskPage.STYLE.contentSmall;
        case DisplaySize.MEDIUM:
          return RiskPage.STYLE.contentMedium;
        case DisplaySize.LARGE:
          return RiskPage.STYLE.contentLarge;
      }
    })();
    return(
      <PageWrapper>
        <div style={containerStyle}>
          <RiskParametersView
            parameters={this.props.parameters}
            currencyDatabase={this.props.currencyDatabase}
            displaySize={this.props.displaySize}/>
          <div style={RiskPage.STYLE.mediumPadding}/>
          <div style={RiskPage.STYLE.lineWrapper}>
            <HLine color={RiskPage.LINE_COLOR}/>
          </div>
          <div style={RiskPage.STYLE.mediumPadding}/>
          <SubmissionBox roles={this.props.roles}/>
        </div>
      </PageWrapper>);
  }
  
  private static readonly STYLE = {
    contentSmall: {
      paddingTop: '30px',
      paddingBottom: '40px',
      paddingLeft: '18px',
      paddingRight: '18px',
      boxSizing: 'border-box' as 'border-box',
      minWidth: '284px',
      flexShrink: 1,
      flexGrow: 1,
      maxWidth: '424px',
      display: 'flex' as 'flex',
      flexDirection: 'column' as 'column',
      justifyContent: 'center' as 'center',
      alignItems: 'center' as 'center'
    },
    contentMedium: {
      paddingTop: '30px',
      paddingBottom: '40px',
      paddingLeft: '18px',
      paddingRight: '18px',
      boxSizing: 'border-box' as 'border-box',
      width: '732px',
      display: 'flex' as 'flex',
      flexDirection: 'column' as 'column',
      justifyContent: 'center' as 'center',
      alignItems: 'center' as 'center'
    },
    contentLarge: {
      paddingTop: '30px',
      paddingBottom: '40px',
      paddingLeft: '18px',
      paddingRight: '18px',
      boxSizing: 'border-box' as 'border-box',
      width: '1000px',
      display: 'flex' as 'flex',
      flexDirection: 'column' as 'column',
      justifyContent: 'center' as 'center',
      alignItems: 'center' as 'center'
    },
    mediumPadding: {
      width: '100%',
      height: '30px'
    },
    lineWrapper: {
      width: '246px'
    }
  }
  private static readonly LINE_COLOR = '#E6E6E6';
}
