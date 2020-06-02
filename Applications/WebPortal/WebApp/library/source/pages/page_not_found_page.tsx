import { HBoxLayout, Padding, VBoxLayout } from 'dali';
import * as React from 'react';
import { DisplaySize } from '../display_size';
import { HLine } from '../components';
import { PageWrapper } from './page_wrapper';

interface Properties {
  displaySize: DisplaySize;
}

export class PageNotFoundPage extends React.Component<Properties> {

  public render(): JSX.Element {
    const headerText = (() => {
      if(this.props.displaySize === DisplaySize.LARGE) {
        return PageNotFoundPage.STYLE.headerTextLarge;
      } else {
        return PageNotFoundPage.STYLE.headerTextSmallMedium;
      }
    })();
    return (
      <PageWrapper>
        <VBoxLayout
            width={PageNotFoundPage.PAGE_WIDTHS[this.props.displaySize]}>
          <Padding size={PageNotFoundPage.LARGE_PADDING}/>
          <div style={PageNotFoundPage.STYLE.centeredWrapper}>
            <img src='resources/page_not_found/broken-page.svg' 
              height='46px' width='38px'/>
          </div>
          <Padding size={PageNotFoundPage.SMALL_PADDING}/>
          <span style={headerText}>Page Not Found</span>
          <Padding size={PageNotFoundPage.SMALL_PADDING}/>
            <HBoxLayout>
              <Padding/>
              <div style={PageNotFoundPage.STYLE.lineWrapper}>
                <HLine height='1px' color='#684BC7'/>
              </div>
              <Padding/>
            </HBoxLayout>
          <Padding size={PageNotFoundPage.SMALL_PADDING}/>
            <div style={PageNotFoundPage.STYLE.bodyText}>
              The page you're looking for can't be found.
            </div> 
          <Padding size={PageNotFoundPage.MEDIUM_PADDING}/>
        </VBoxLayout>
      </PageWrapper>);
  }
  private static readonly SMALL_PADDING = '30px';
  private static readonly MEDIUM_PADDING = '60px';
  private static readonly LARGE_PADDING = '150px';
  private static readonly PAGE_WIDTHS = {
    [DisplaySize.SMALL]: '218px',
    [DisplaySize.MEDIUM]: '218px',
    [DisplaySize.LARGE]: '316px'
  };
  private static readonly STYLE = {
    headerTextSmallMedium: {
      fontWeight: 700,
      fontFamily: 'Roboto',
      fontSize: '30px',
      color: '#684BC7',
      textAlign: 'center'
    } as React.CSSProperties,
    headerTextLarge: {
      fontWeight: 700,
      fontFamily: 'Roboto',
      fontSize: '40px',
      color: '#684BC7',
      textAlign: 'center'
    } as React.CSSProperties,
    bodyText : {
      font: '400 16px Roboto',
      color: '#333333',
      lineHeight: '20px',
      textAlign: 'center'
    } as React.CSSProperties,
    lineWrapper: {
      width: '50px'
    } as React.CSSProperties,
    centeredWrapper: {
      display: 'flex',
      alignItems: 'center',
      justifyContent: 'center'
    } as React.CSSProperties,
  };
}