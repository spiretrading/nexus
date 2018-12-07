import * as React from 'react';
import { DisplaySize , Padding, VBoxLayout, HLine } from '../../..';
import { HBoxLayout } from '../../../layouts';

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
    this.closeUploader = this.closeUploader.bind(this);
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
        <ChangePictureModal displaySize={this.props.displaySize}
          visibility={this.state.showUploader}
          closeModal={this.closeUploader}/>
      </div>);
  }

  private showUploader() {
    this.setState({ showUploader: true });
    this.props.onUpload();
  }

  private closeUploader() {
    this.setState({ showUploader: false });
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

interface ModalProperties {
  displaySize: DisplaySize;
  visibility: boolean;
  closeModal: () => void;
}

/** Displays an account's profile page. */
export class ChangePictureModal extends React.Component<ModalProperties> {
  public render(): JSX.Element {
    const visibility = (() => {
      if(this.props.visibility) {
        return ChangePictureModal.STYLE.wrapper;
      } else {
        return ChangePictureModal.STYLE.hidden;
      }
    })();
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
    const buttonBox = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return ChangePictureModal.STYLE.buttonBoxSmall;
      } else {
        return ChangePictureModal.STYLE.buttonBoxLarge;
      }
    })();
    const imageStyle = (() => {
      switch (this.props.displaySize) {
        case DisplaySize.SMALL:
          return ChangePictureModal.STYLE.imageSmall;
        case DisplaySize.MEDIUM:
          return ChangePictureModal.STYLE.imageLagre;
        case DisplaySize.LARGE:
          return ChangePictureModal.STYLE.imageLagre;
      }
    })();
    return (
      <div style={visibility}>
        <HBoxLayout style={boxStyle}>
          <Padding size={ChangePictureModal.PADDING}/>
          <VBoxLayout>
            <Padding size={ChangePictureModal.PADDING}/>
            <div style={ChangePictureModal.STYLE.header}>
              {ChangePictureModal.HEADER_TEXT}
              <img src='resources/close.svg'
                style={ChangePictureModal.STYLE.closeIcon}
                onClick={this.props.closeModal}/>
            </div>
            <Padding size={ChangePictureModal.PADDING_ELEMENT}/>
            <img src={ChangePictureModal.SOME_IMAGE}
                style={imageStyle}/>
            <Padding size={ChangePictureModal.PADDING_ELEMENT}/>
            <div style={ChangePictureModal.STYLE.tempSlider}/>
            <Padding size={ChangePictureModal.PADDING_ELEMENT}/>
            <HLine color='#E6E6E6'/>
            <Padding size={ChangePictureModal.PADDING_ELEMENT}/>
            <div style={buttonBox}>
              <button style={ChangePictureModal.STYLE.buttonStyle}>
                {ChangePictureModal.BROWSE_BUTTON_TEXT}
              </button>
              <button style={ChangePictureModal.STYLE.buttonStyle}>
                {ChangePictureModal.SUBMIT_BUTTON_TEXT}
              </button>
            </div>
            <Padding size={ChangePictureModal.PADDING}/>
          </VBoxLayout>
          <Padding size={ChangePictureModal.PADDING}/>
        </HBoxLayout>
      </div>);
  }
  private static readonly STYLE = {
    wrapper: {
      boxSizing: 'border-box' as 'border-box',
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
    header: {
      display: 'flex' as 'flex',
      justifyContent: 'space-between'  as 'space-between',
      font: '400 16px Roboto'
    },
    closeIcon: {
      width: '20px',
      height: '20px',
      cursor: 'pointer' as 'pointer'
    },
    tempSlider: {
      width: '100%',
      height: '20px',
      backgroundColor: '#967FE3'
    },
    buttonBoxSmall: {
      boxSizing: 'border-box' as 'border-box',
      display: 'flex' as 'flex',
      flexDirection: 'column' as 'column',
      flexWrap: 'wrap' as 'wrap',
      alignItems: 'center' as 'center',
      justifyContent: 'space-between' as 'space-between',
      height: '86px'
    },
    buttonBoxLarge: {
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      flexWrap: 'wrap' as 'wrap',
      justifyContent: 'space-between' as 'space-between',
      alignItems: 'center' as 'center'
    },
    buttonStyle: {
      minWidth: '153px',
      maxWidth: '248px',
      height: '34px',
      backgroundColor: '#684BC7',
      color: '#FFFFFF',
      font: '400 14px Roboto',
      border:'1px solid #684BC7',
      borderRadius: '1px',
      outline: 0
    },
    imageSmall: {
      objectFit: 'cover' as 'cover',
      height: '166px',
      width: '248px'
    },
    imageLagre: {
      objectFit: 'cover' as 'cover',
      height: '216px',
      width: '324px'
    },
    hidden: {
      visibility: 'hidden' as 'hidden',
      display: 'none' as 'none'
    }
  };
  private static readonly HEADER_TEXT = 'Change Picture';
  private static readonly BROWSE_BUTTON_TEXT = 'BROWSE';
  private static readonly SUBMIT_BUTTON_TEXT = 'SUBMIT';
  private static readonly PADDING = '18px';
  private static readonly PADDING_ELEMENT = '30px';
  private static readonly SOME_IMAGE = 'https://upload.wikimedia.org/' +
  'wikipedia/commons/thumb/2/23/Close_up_of_a_black_domestic_cat.jpg/' +
  '675px-Close_up_of_a_black_domestic_cat.jpg';
}
