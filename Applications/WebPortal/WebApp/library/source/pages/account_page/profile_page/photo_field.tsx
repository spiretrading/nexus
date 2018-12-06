import * as React from 'react';
import { DisplaySize } from '../../..';

export enum DisplayMode {
  Display,
  Uploading
}

interface Properties {
  displaySize: DisplaySize;
  displayMode?: DisplayMode;
  imageSource?: string;
  readonly?: boolean;
  onUpload?: () => boolean;
}

/** Displays an account's profile page. */
export class PhotoField extends React.Component<Properties> {
  public static readonly defaultProps = {
    readonly: false,
    onClick: () => {true;},
    DisplayMode: DisplayMode.Display
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
    const cameraIconStyle = (() => {
      if(this.props.readonly) {
        return PhotoField.STYLE.hidden;
      } else {
        return PhotoField.STYLE.cameraIcon;
      }
    })();
    const imageStyle = (() => {
      if(!this.props.imageSource) {
        if(this.props.displaySize === DisplaySize.SMALL) {
          return PhotoField.STYLE.placeholderStyleSmall;
        } else {
          return PhotoField.STYLE.placeholderStyle;
        }
      } else {
        if(this.props.displaySize === DisplaySize.SMALL) {
          return PhotoField.STYLE.imageStyleSmall;
        } else {
          return PhotoField.STYLE.imageStyle;
        }
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
      <div style={PhotoField.STYLE.wrapper}>
        <div style={boxStyle}>
          <img src={imageSrc}
            style={imageStyle}/>
          <img src='resources/account_page/profile_page/camera.svg'
            style={cameraIconStyle}
            onClick={this.props.onUpload}/>
        </div>
      </div>);
  }
  private static readonly STYLE = {
    wrapper: {
      maxHeight: '288px',
      maxWidth: '424px'
    },
    boxSmall: {
      boxSizing: 'border-box' as 'border-box',
      backgroundColor: '#F8F8F8',
      width: '100%',
      paddingTop: '68%',
      maxHeight: '288px',
      maxWidth: '424px',
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
      height: '24px',
      width: '30px'
    },
    placeholderStyleSmall: {
      position: 'absolute' as 'absolute',
      height: '24px',
      width: '30px',
      top: 'calc(50% - 12px)',
      left: 'calc(50% - 15px)'
    },
    imageStyle: {
      height: '100%',
      width: '100%'
    },
    imageStyleSmall: {
      position: 'absolute' as 'absolute',
      top: '0%',
      left: '0%',
      height: '100%',
      width: '100%'
    },
    cameraIcon: {
      position: 'absolute' as 'absolute',
      height: '24px',
      width: '24px',
      top: 'calc(0% + 10px)',
      left: 'calc(100% - 10px - 24px)',
      cursor: 'pointer' as 'pointer'
    },
    hidden: {
      visibility: 'hidden' as 'hidden',
      display: 'none' as 'none'
    }
  };
}
