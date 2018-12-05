import * as React from 'react';
import { DisplaySize } from '../../..';
import { callbackify } from 'util';

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
      switch (this.props.displaySize) {
        case DisplaySize.SMALL:
          return PhotoField.STYLE.boxSmall;
        case DisplaySize.MEDIUM:
          return PhotoField.STYLE.boxMedium;
        case DisplaySize.LARGE:
          return PhotoField.STYLE.boxLarge;
      }
    })();
    const imageStyle = (() => {
      if (!this.props.imageSource) {
        if (this.props.displaySize === DisplaySize.SMALL) {
          return PhotoField.STYLE.placeholderStyleSmall;
        }
        return PhotoField.STYLE.placeholderStyle;
      } else {
        return PhotoField.STYLE.imageStyle
      }
    })();
    const imageSrc = (() => {
      if (!this.props.imageSource) {
        return 'resources/account_page/profile_page/image-placeholder.svg';
      } else {
        return this.props.imageSource;
      }
    })();
    return (
        <div id='dumb box' style={boxStyle}>
          <img src={imageSrc}
            style={imageStyle} />
          <img src='resources/account_page/profile_page/camera.svg'
              style={PhotoField.STYLE.cameraIcon}/>
        </div>);
  }
  private static STYLE = {
    boxSmall: {
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      alignItems: 'center' as 'center',
      justifyContent: 'center' as 'center',
      backgroundColor: '#F8F8F8',
      height: '190px',
      width: '284px',
      position: 'relative' as 'relative'
    },
    boxMedium: {
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      alignItems: 'center' as 'center',
      justifyContent: 'center' as 'center',
      backgroundColor: '#F8F8F8',
      border: '1px solid #E6E6E6',
      height: '190px',
      width: '284px',
      position: 'relative' as 'relative'
    },
    boxLarge: {
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      alignItems: 'center' as 'center',
      justifyContent: 'center' as 'center',
      backgroundColor: '#F8F8F8',
      border: '1px solid #E6E6E6',
      height: '258px',
      width: '380px',
      position: 'relative' as 'relative'
    },
    placeholderStyle: {
      position: 'absolute' as 'absolute',
      height: '100px',
      width: '100px'
    },
    placeholderStyleSmall: {
      flexGrow: '100',
      position: 'absolute' as 'absolute',
      height: '24px',
      width: '30px',
      top: '46%',
      left: '45%'
    },
    imageStyle: {
      height: '100%',
      width: '100%'
    },
    cameraIcon: {
      position: 'absolute' as 'absolute',
      height: '24px',
      width: '24px',
      top: 'calc(0% + 10px)',
      left: 'calc(100% - 10px - 24px)'
    }
  };
}
