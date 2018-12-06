import * as React from 'react';
import { DisplaySize } from '../../..';
import { callbackify } from 'util';
import { VBoxLayout } from '../../../layouts';

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

interface State {
  showUploader: boolean;
}

/** Displays an account's profile page. */
export class PhotoField extends React.Component<Properties, State> {
  public static readonly defaultProps = {
    readonly: false,
    onClick: () => {true;},
    DisplayMode: DisplayMode.Display
  };

  constructor(props: Properties) {
    super(props);
    this.state = {
      showUploader: false
    };
    this.showUploader = this.showUploader.bind(this);
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
    const uploaderStyle = (() => {
      if(!this.state.showUploader) {
        return PhotoField.STYLE.hidden;
      } else {
        return null;
      }
    })();
    return (
      <div style={PhotoField.STYLE.wrapper}>
        <div style={boxStyle}>
          <img src={imageSrc}
            style={imageStyle}/>
          <img src='resources/account_page/profile_page/camera.svg'
            style={cameraIconStyle}
            onClick={this.showUploader}/>
        </div>
        <ChangePictureModal displaySize={this.props.displaySize}/>
      </div>);
  }

  private showUploader() {
    this.setState({ showUploader: true });
    this.props.onUpload();
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

/** Displays an account's profile page. */
export class ChangePictureModal extends React.Component<Properties> {
  public render(): JSX.Element {
    const boxStyle = (() => {
      switch (this.props.displaySize) {
        case DisplaySize.SMALL:
          return ChangePictureModal.STYLE.boxSmall;
        case DisplaySize.MEDIUM:
          return ChangePictureModal.STYLE.boxLarge;
        case DisplaySize.LARGE:
          return ChangePictureModal.STYLE.boxLarge;
      }
    })();
    return (
      <div>
      <div style={ChangePictureModal.STYLE.wrapper}>
        <VBoxLayout style={boxStyle}>
          BEEP
        </VBoxLayout>
      </div>
      </div>);
  }
  private static readonly STYLE = {
    wrapper: {
      top: '0',
      left: '0',
      position: 'fixed' as 'fixed',
      width: '100%',
      height: '100%',
      zIndex: 100,
      backgroundColor: '#FFFFFFF2'
    },
    boxSmall: {
      position: 'absolute' as 'absolute',
      zIndex: 5,
      border: '1px solid #FFFFFF',
      boxShadow: '0px 0px 6px #00000064',
      backgroundColor: '#FFFFFF',
      width: '284px',
      height: '100%',
      top: '0%',
      right: '0%',
      opacity: 1
    },
    boxLarge: {
      position: 'absolute' as 'absolute',
      backgroundColor: '#FFFFFF',
      width: '360px',
      height: '447px',
      boxShadow: '0px 0px 6px #00000064',
      top: 'calc(50% - 223.5px)',
      left: 'calc(50% - 180px)',
      opacity: 1
    },
    closeIcon: {
      width: '20px',
      height: '20px'
    },
    buttonBoxStyle: {
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      flexWrap: 'wrap' as 'wrap'
    }, 
    buttonStyle: {
    },
    hidden: {
      visibility: 'hidden' as 'hidden',
      display: 'none' as 'none'
    }
  };
}
