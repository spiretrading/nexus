import { css, StyleSheet } from 'aphrodite/no-important';
import * as React from 'react';
import { Transition } from 'react-transition-group';
import { DisplaySize, HBoxLayout, Padding, VBoxLayout, HLine } from '../../..';

export enum DisplayMode {
  Display,
  Uploading
}

interface Properties {
  displaySize: DisplaySize;
  displayMode: DisplayMode;
  imageSource?: string;
  readonly?: boolean;
  scaling?: number;
  onToggleUploader: () => void;
  onUpload: (newFileLocation: string, scaling: number) => void;
}

/** Displays an account's profile image */
export class PhotoField extends React.Component<Properties, {}> {
  public static readonly defaultProps = {
    readonly: false,
    DisplayMode: DisplayMode.Display,
    scaling: 0
  };

  constructor(props: Properties) {
    super(props);
    this.showChangePictureModal = this.showChangePictureModal.bind(this);
    this.hideChangePictureModal = this.hideChangePictureModal.bind(this);
  }

  public render(): JSX.Element {
    const boxStyle = (() => {
      switch(this.props.displaySize) {
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
    const imageSrc = (() => {
      if(!this.props.imageSource) {
        return 'resources/account_page/profile_page/image-placeholder.svg';
      } else {
        return this.props.imageSource;
      }
    })();
    const imageStyle = (() => {
      if(!this.props.imageSource) {
        if(this.props.displaySize === DisplaySize.SMALL) {
          return PhotoField.STYLE.placeholderSmall;
        } else {
          return PhotoField.STYLE.placeholder;
        }
      } else {
        if(this.props.displaySize === DisplaySize.SMALL) {
          return PhotoField.STYLE.imageSmall;
        } else {
          return PhotoField.STYLE.image;
        }
      }
    })();
    const imageScaling = (() => {
      if(this.props.imageSource) {
        return ({
          transform: `scale(${(100 + this.props.scaling) / 100})`
        });
      } else {
        return ({ transform: `scale(1)` });
      }
    })();
    return (
      <div style={PhotoField.STYLE.wrapper}>
        <div style={boxStyle}>
          <img src={imageSrc}
            style={{...imageStyle, ...imageScaling}}/>
          <div style={PhotoField.STYLE.imageBoxSmall}>
            <img src='resources/account_page/profile_page/camera.svg'
              style={cameraIconStyle}
              onClick={this.props.onToggleUploader}/>
          </div>
        </div>
        <Transition in={this.props.displayMode === DisplayMode.Uploading}
            timeout={PhotoField.TIMEOUT}>
          {(state) => (
            <div style={{
              ...PhotoField.STYLE.animationBase,
              ...(PhotoField.ANIMATION_STYLE as any)[state]
            }}>
              <ChangePictureModal displaySize={this.props.displaySize}
                imageSource={this.props.imageSource}
                onCloseModal={this.props.onToggleUploader}
                onSubmitImage={this.props.onUpload}/>
            </div>)}
        </Transition>
      </div>);
  }

  private showChangePictureModal() {
    this.setState({ showUploader: true });
  }

  private hideChangePictureModal() {
    this.setState({ showUploader: false });
  }
  private static ANIMATION_STYLE = {
    entering: {
      opacity: 0
    },
    entered: {
      opacity: 1
    },
    exited: {
      display: 'none' as 'none'
    }
  };
  private static readonly STYLE = {
    wrapper: {
      maxHeight: '288px',
      maxWidth: '424px'
    },
    animationBase: {
      opacity: 0,
      transition: 'opacity 200ms ease'
    },
    boxSmall: {
      boxSizing: 'border-box' as 'border-box',
      backgroundColor: '#F8F8F8',
      width: '100%',
      paddingTop: '68%',
      maxHeight: '288px',
      maxWidth: '424px',
      position: 'relative' as 'relative',
      borderRadius: '1px',
      border: '1px solid #EBEBEB',
      overflow: 'hidden' as 'hidden'
    },
    boxMedium: {
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      alignItems: 'center' as 'center',
      justifyContent: 'center' as 'center',
      backgroundColor: '#F8F8F8',
      border: '1px solid #E6E6E6',
      borderRadius: '1px',
      height: '190px',
      width: '284px',
      position: 'relative' as 'relative',
      overflow: 'hidden' as 'hidden'
    },
    boxLarge: {
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      alignItems: 'center' as 'center',
      justifyContent: 'center' as 'center',
      backgroundColor: '#F8F8F8',
      border: '1px solid #E6E6E6',
      borderRadius: '1px',
      height: '258px',
      width: '380px',
      position: 'relative' as 'relative',
      overflow: 'hidden' as 'hidden'
    },
    placeholder: {
      position: 'absolute' as 'absolute',
      height: '24px',
      width: '30px'
    },
    placeholderSmall: {
      position: 'absolute' as 'absolute',
      height: '24px',
      width: '30px',
      top: 'calc(50% - 12px)',
      left: 'calc(50% - 15px)'
    },
    imageBoxSmall: {
      boxSizing: 'border-box' as 'border-box',
      overflow: 'hidden' as 'hidden'
    },
    imageBoxMedium: {
      boxSizing: 'border-box' as 'border-box',
      overflow: 'hidden' as 'hidden'
    },
    imageBoxLarge: {
      boxSizing: 'border-box' as 'border-box',
      overflow: 'hidden' as 'hidden'
    },
    image: {
      objectFit: 'cover' as 'cover',
      height: '100%',
      width: '100%'
    },
    imageSmall: {
      position: 'absolute' as 'absolute',
      objectFit: 'cover' as 'cover',
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
  private static readonly TIMEOUT = 200;
}

interface ModalProperties {
  imageSource?: string;
  displaySize: DisplaySize;
  onCloseModal: () => void;
  onSubmitImage: (newFileLocation: string, scaling: number) => void;
}

interface ModalState {
  imageScaling: number;
  currentImage: string;
}

/** Displays a modal that allows the user to change their picture. */
export class ChangePictureModal extends
  React.Component<ModalProperties, ModalState> {
  constructor(properties: ModalProperties) {
    super(properties);
    this.state = {
      imageScaling: 0,
      currentImage: this.props.imageSource
    };
    this.onSliderMovement = this.onSliderMovement.bind(this);
    this.onSubmit = this.onSubmit.bind(this);
    this.onGetImageFile = this.onGetImageFile.bind(this);
    this.onClose = this.onClose.bind(this);
  }

  public render(): JSX.Element {
    const boxStyle = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return ChangePictureModal.STYLE.boxSmall;
      } else {
        return ChangePictureModal.STYLE.boxLarge;
      }
    })();
    const boxShadowStyle = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return ChangePictureModal.STYLE.boxShadowSmall;
      } else {
        return ChangePictureModal.STYLE.boxShadowLarge;
      }
    })();
    const buttonBox = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return ChangePictureModal.STYLE.buttonBoxSmall;
      } else {
        return ChangePictureModal.STYLE.buttonBoxLarge;
      }
    })();
    const imageSrc = (() => {
      if(!this.state.currentImage) {
        return 'resources/account_page/profile_page/image-placeholder.svg';
      } else {
        return this.state.currentImage;
      }
    })();
    const imageStyle = (() => {
      if(!this.state.currentImage) {
        return ChangePictureModal.STYLE.placeholderImage;
      } else if(this.props.displaySize === DisplaySize.SMALL) {
        return ChangePictureModal.STYLE.imageSmall;
      } else {
        return ChangePictureModal.STYLE.imageLarge;
      }
    })();
    const imageBoxStyle = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return ChangePictureModal.STYLE.imageBoxSmall;
      } else {
        return ChangePictureModal.STYLE.imageBoxLarge;
      }
    })();
    const isSliderReadOnly = (() => {
      if(this.props.imageSource) {
        return false;
      } else {
        return true;
      }
    })();
    const imageScaling = (() => {
      if(this.props.imageSource) {
        return ({
          transform: `scale(${(100 + this.state.imageScaling) / 100})`
        });
      } else {
        return ({ transform: `scale(1)` });
      }
    })();
    return (
      <div>
        <div style={ChangePictureModal.STYLE.transparentBackground}/>
        <div style={boxShadowStyle}/>
        <HBoxLayout style={boxStyle}>
          <Padding size={ChangePictureModal.PADDING}/>
          <VBoxLayout>
            <Padding size={ChangePictureModal.PADDING}/>
            <div style={ChangePictureModal.STYLE.header}>
              {ChangePictureModal.HEADER_TEXT}
              <img src='resources/close.svg'
                style={ChangePictureModal.STYLE.closeIcon}
                onClick={this.onClose}/>
            </div>
            <Padding size={ChangePictureModal.PADDING_BETWEEN_ELEMENTS}/>
            <div style={imageBoxStyle}>
              <img src={imageSrc}
                style={{ ...imageStyle, ...imageScaling }}/>
            </div>
            <Padding size={ChangePictureModal.PADDING_BETWEEN_ELEMENTS}/>
            <Slider onInput={this.onSliderMovement}
              scaleValue={this.state.imageScaling}
              isReadOnly={isSliderReadOnly}/>
            <Padding size={ChangePictureModal.PADDING_BETWEEN_ELEMENTS}/>
            <HLine color='#E6E6E6' height={1}/>
            <Padding size={ChangePictureModal.PADDING_BETWEEN_ELEMENTS}/>
            <div style={buttonBox}>
              <input type='file' id='imageInput' accept='image/*'
                style={ChangePictureModal.STYLE.hiddenInput}
                onChange={(e) => { this.onGetImageFile(e.target.files); }}/>
              <label htmlFor='imageInput'
                className={css(ChangePictureModal.SPECIAL_STYLE.button)}>
                {ChangePictureModal.BROWSE_BUTTON_TEXT}
              </label>
              <div className={css(ChangePictureModal.SPECIAL_STYLE.button)}
                onClick={this.onSubmit}>
                {ChangePictureModal.SUBMIT_BUTTON_TEXT}
              </div>
            </div>
            <Padding size={ChangePictureModal.PADDING}/>
          </VBoxLayout>
          <Padding size={ChangePictureModal.PADDING}/>
        </HBoxLayout>
      </div>);
  }

  private onSliderMovement(value: number) {
    this.setState({ imageScaling: value });
  }

  private onGetImageFile(selectorFiles: FileList) {
    const file = selectorFiles.item(0);
    const someURL = URL.createObjectURL(file);
    this.setState({
      currentImage: someURL,
      imageScaling: 0
      });
  }

  private onClose() {
    this.props.onCloseModal();
    this.setState({ imageScaling: 0 });
    this.setState({ currentImage: this.props.imageSource });
  }

  private onSubmit() {
    if(this.state.currentImage) {
      this.props.onSubmitImage(this.state.currentImage,
        this.state.imageScaling);
    }
    this.props.onCloseModal();
  }

  private static readonly STYLE = {
    transparentBackground: {
      boxSizing: 'border-box' as 'border-box',
      top: '0',
      left: '0',
      position: 'fixed' as 'fixed',
      width: '100%',
      height: '100%',
      zIndex: 80,
      backgroundColor: '#FFFFFF',
      opacity: 0.9
    },
    boxShadowSmall:{
      opacity: 0.4,
      display: 'block',
      boxShadow: '0px 0px 6px #000000',
      position: 'absolute' as 'absolute',
      zIndex: 100,
      border: '1px solid #FFFFFF',
      backgroundColor: '#FFFFFF',
      width: '284px',
      height: '100%',
      top: '0%',
      right: '0%'
    },
    boxShadowLarge:{
      opacity: 0.4,
      boxShadow: '0px 0px 6px #000000',
      zIndex: 100,
      display: 'block',
      position: 'absolute' as 'absolute',
      backgroundColor: '#FFFFFF',
      width: '360px',
      height: '447px',
      top: 'calc(50% - 223.5px)',
      left: 'calc(50% - 180px)'
    },
    boxSmall: {
      display: 'block',
      position: 'absolute' as 'absolute',
      zIndex: 101,
      border: '1px solid #FFFFFF',
      backgroundColor: '#FFFFFF',
      width: '284px',
      height: '100%',
      top: '0%',
      right: '0%'
    },
    boxLarge: {
      zIndex: 101,
      display: 'block',
      position: 'absolute' as 'absolute',
      backgroundColor: '#FFFFFF',
      width: '360px',
      height: '447px',
      top: 'calc(50% - 223.5px)',
      left: 'calc(50% - 180px)'
    },
    header: {
      display: 'flex' as 'flex',
      justifyContent: 'space-between' as 'space-between',
      font: '400 16px Roboto'
    },
    closeIcon: {
      width: '20px',
      height: '20px',
      cursor: 'pointer' as 'pointer'
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
      border: '1px solid #684BC7',
      borderRadius: '1px',
      outline: 0
    },
    placeholderImage: {
      position: 'relative' as 'relative',
      height: '24px',
      width: '30px',
      top: 'calc(50% - 12px)',
      left: 'calc(50% - 15px)'
    },
    imageSmall: {
      objectFit: 'cover' as 'cover',
      height: '100%',
      width: '100%'
    },
    imageLarge: {
      objectFit: 'cover' as 'cover',
      height: '100%',
      width: '100%'
    },
    imageBoxSmall: {
      boxSizing: 'border-box' as 'border-box',
      height: '166px',
      width: '248px',
      overflow: 'hidden' as 'hidden',
      borderRadius: '1px',
      border: '1px solid #EBEBEB',
      backgroundColor: '#F8F8F8'
    },
    imageBoxLarge: {
      boxSizing: 'border-box' as 'border-box',
      height: '216px',
      width: '324px',
      overflow: 'hidden' as 'hidden',
      borderRadius: '1px',
      border: '1px solid #EBEBEB',
      backgroundColor: '#F8F8F8'
    },
    hiddenInput: {
      width: '0.1px',
      height: '0.1px',
      opacity: 0,
      overflow: 'hidden' as 'hidden',
      position: 'absolute' as 'absolute',
      zIndex: -1
    }
  };
  private static readonly SPECIAL_STYLE = StyleSheet.create({
    button: {
      boxSizing: 'border-box' as 'border-box',
      cursor: 'pointer' as 'pointer',
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      flexWrap: 'wrap' as 'wrap',
      justifyContent: 'center' as 'center',
      alignItems: 'center' as 'center',
      minWidth: '153px',
      maxWidth: '248px',
      height: '34px',
      backgroundColor: '#684BC7',
      color: '#FFFFFF',
      font: '400 14px Roboto',
      border: '1px solid #684BC7',
      borderRadius: '1px',
      outline: 0,
      ':active': {
        backgroundColor: '#4B23A0'
      },
      ':hover': {
        backgroundColor: '#4B23A0'
      }
    }
  });
  private static readonly HEADER_TEXT = 'Change Picture';
  private static readonly BROWSE_BUTTON_TEXT = 'BROWSE';
  private static readonly SUBMIT_BUTTON_TEXT = 'SUBMIT';
  private static readonly PADDING = '18px';
  private static readonly PADDING_BETWEEN_ELEMENTS = '30px';
}

interface SliderProperties {
  onInput?: (num: number) => void;
  isReadOnly?: boolean;
  scaleValue?: number;
}

export class Slider extends React.Component<SliderProperties, {}> {
  public static readonly defaultProps = {
    onSliderMove: () => {},
    scaleValue: 0,
    isReadOnly: false
  };

  constructor(properties: SliderProperties) {
    super(properties);
    this.onInput = this.onInput.bind(this);
  }

  public render(): JSX.Element {
    return (
      <input type='range'
        min={Slider.MIN_RANGE_VALUE}
        max={Slider.MAX_RANGE_VALUE}
        value={this.props.scaleValue}
        disabled={this.props.isReadOnly}
        onChange={this.onInput}
        className={css(Slider.SLIDER_STYLE.slider)}/>);
  }

  private onInput(event: any) {
    const num = event.target.value;
    const diff = Math.abs(this.props.scaleValue - num);
    if(this.props.scaleValue < num) {
      this.props.onInput(this.props.scaleValue + diff);
    } else {
      this.props.onInput(this.props.scaleValue - diff);
    }
  }

  public static readonly SLIDER_STYLE = StyleSheet.create({
    slider: {
      width: '100%',
      height: '20px',
      margin: 0,
      outline: 0,
      '::-webkit-slider-thumb': {
        '-webkit-appearance': 'none',
        boxSizing: 'border-box' as 'border-box',
        cursor: 'pointer' as 'pointer',
        height: '20px',
        width: '20px',
        backgroundColor: '#FFFFFF',
        border: '1px solid #8C8C8C',
        borderRadius: '20px',
        boxShadow: 'none',
        marginTop: '-8px'
      },
      '::-moz-range-thumb': {
        boxSizing: 'border-box' as 'border-box',
        cursor: 'pointer' as 'pointer',
        height: '20px',
        width: '20px',
        backgroundColor: '#FFFFFF',
        border: '1px solid #8C8C8C',
        borderRadius: '20px'
      },
      '::-ms-thumb': {
        boxSizing: 'border-box' as 'border-box',
        cursor: 'pointer' as 'pointer',
        height: '20px',
        width: '20px',
        backgroundColor: '#FFFFFF',
        border: '1px solid #8C8C8C',
        borderRadius: '20px',
        marginTop: '0px'
      },
      '::-webkit-slider-runnable-track': {
        '-webkit-appearance': 'none',
        boxShadow: 'none' as 'none',
        backgroundColor: '#E6E6E6',
        height: '4px'
      },
      '::-moz-range-track': {
        backgroundColor: '#E6E6E6',
        height: '4px',
        border: 0
      },
      '::-ms-track': {
        backgroundColor: '#E6E6E6',
        height: '4px'
      },
      '-webkit-appearance': 'none',
      '::-moz-focus-outer': {
        border: 0
      }
    }
  });
  private static readonly MIN_RANGE_VALUE = 0;
  private static readonly MAX_RANGE_VALUE = 200;
}
