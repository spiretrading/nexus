import { HBoxLayout, Padding, VBoxLayout } from 'dali';
import * as React from 'react';
import { HLine } from '../components';
import { DisplaySize } from '../display_size';
import { PageWrapper } from './page_wrapper';

interface Properties {
  
  /* The size of the viewport. */
  displaySize: DisplaySize;
}

/* Displays a page that indicates the desired page cannot be found. */
export class PageNotFoundPage extends React.Component<Properties> {

  public render(): JSX.Element {
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
          <span style={PageNotFoundPage.HEADER_STYLE[this.props.displaySize]}>
            Page Not Found
          </span>
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
  private static readonly HEADER_STYLE = {
    [DisplaySize.SMALL]: {
      fontWeight: 700,
      fontFamily: 'Roboto',
      fontSize: '30px',
      color: '#684BC7',
      textAlign: 'center'
    } as React.CSSProperties,
    [DisplaySize.MEDIUM]: {
      fontWeight: 700,
      fontFamily: 'Roboto',
      fontSize: '30px',
      color: '#684BC7',
      textAlign: 'center'
    } as React.CSSProperties,
    [DisplaySize.LARGE]: {
      fontWeight: 700,
      fontFamily: 'Roboto',
      fontSize: '40px',
      color: '#684BC7',
      textAlign: 'center'
    } as React.CSSProperties
  };
  private static readonly STYLE = {
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
    } as React.CSSProperties
  };
}
