import * as React from 'react';
import { DisplaySize } from '../../..';
import { FormEntry } from './form_entry';

export enum DisplayMode {
  Display,
  Uploading
}

interface Properties {
  displaySize: DisplaySize;
  displayMode?: DisplayMode;
  imageSource?: string;
  readonly?: boolean;
}

/** Displays an account's profile page. */
export class PhotoField extends React.Component<Properties> {
  constructor(props: Properties) {
    super(props);
  }

  public render(): JSX.Element {
    const boxStyle = (() => {
      switch(this.props.displaySize){
        case DisplaySize.SMALL:
          return PhotoField.STYLE.boxSmall;
        case DisplaySize.MEDIUM:
          return PhotoField.STYLE.boxMedium;
        case DisplaySize.LARGE:
          return PhotoField.STYLE.boxLarge;
      }
    })();
    const imageStyle = (() => {
      if(!this.props.imageSource) {
        return PhotoField.STYLE.placeholderStyle;
      } else {
        return null;
      }
    })();
    const imageSrc = (() => {
      if(!this.props.imageSource) {
        return 'resources/account_page/profile_page/image-placeholder.svg';
      } else {
        return this.props.imageSource;
      }
    })();
    return (
      <div style={boxStyle}>
        <img src='resources/account_page/profile_page/image-placeholder.svg' 
        style={imageStyle}/>
      </div>
    );
  }
  private static STYLE = {
    boxSmall: {
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      alignItems: 'center' as 'center',
      justifyContent: 'center' as 'center',
      backgroundColor: '#F8F8F8',
      border: '1px solid #E6E6E6',
      minHeight: '190px',
      //maxHeight: '288px',
      minWidth: '284px'
      //maxWidth: '424px'
    },
    boxMedium: {
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      alignItems: 'center' as 'center',
      backgroundColor: '#F8F8F8',
      border: '1px solid #E6E6E6',
      height: '190px',
      width: '284px'
    },
    boxLarge: {
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      alignItems: 'center' as 'center',
      backgroundColor: '#F8F8F8',
      border: '1px solid #E6E6E6',
      height: '258px',
      width: '380px'
    },
    placeholderStyle: {
      height: '24px',
      width: '30px'
    }
  };
}
