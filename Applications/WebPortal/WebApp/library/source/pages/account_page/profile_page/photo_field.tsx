import { css, StyleSheet } from 'aphrodite/no-important';
import { HBoxLayout, Padding, VBoxLayout } from 'dali';
import * as React from 'react';
import { Transition } from 'react-transition-group';
import { DisplaySize, HLine } from '../../..';

/** The modes that the PhotoField can be displayed at. */
export enum PhotoFieldDisplayMode {

  /** Only the photo is visible. */
  DISPLAY,

  /** The uploader is visible. */
  UPLOADING
}

interface Properties {

  /** Determines the size to render the component at. */
  displaySize: DisplaySize;

  /** Determines if the ChangePictureModal is visible or not. */
  displayMode: PhotoFieldDisplayMode;

  /** The URL the image is located at. */
  imageSource?: string;

  /** Determines if the image can be changed or not. */
  readonly?: boolean;

  /** A value that determines how zoomed in the image will be.
   * It is a normalized scalar value.
   */
  scaling: number;

  /** The image displayed in the modal. A temporary image until the submit
   * button is clicked.
   */
  newImageSource: string;

  /** A value that determines how zoomed in the new image will be.
   * It is a normalized scalar value.
   */
  newScaling: number;

  /** Callback to hide or show the uploader. */
  onToggleUploader?: () => void;

  /** Callback to store the file and the scaling for the file. */
  onSubmit?: (newFileLocation: string, scaling: number) => void;

  /** Called to update the photo in the modal. */
  onNewPhotoChange?: (photo: string) => void;

  /** Called to update the scaling of the photo in the modal. */
  onNewScalingChange?: (scale: number) => void;
}

/** Displays an account's profile image. */
export class PhotoField extends React.Component<Properties, {}> {
  public static readonly defaultProps = {
    readonly: false,
    onToggleUploader: () => {},
    onSubmit: () => {},
    onNewPhotoChange: () => {},
    onNewScalingChange: () => {}
  };

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
    const cameraIconWrapper = (() => {
      if(this.props.readonly) {
        return PhotoField.STYLE.hidden;
      } else {
        return PhotoField.STYLE.cameraIconWrapper;
      }
    })();
    const imageSrc = (() => {
      if(this.props.imageSource) {
        return this.props.imageSource;
      } else {
        return 'resources/account_page/profile_page/photo_field/' +
          'image-placeholder.svg';
      }
    })();
    const imageStyle = (() => {
      if(this.props.imageSource) {
        if(this.props.displaySize === DisplaySize.SMALL) {
          return PhotoField.STYLE.imageSmall;
        } else {
          return PhotoField.STYLE.image;
        }
      } else {
        if(this.props.displaySize === DisplaySize.SMALL) {
          return PhotoField.STYLE.placeholderSmall;
        } else {
          return PhotoField.STYLE.placeholder;
        }
      }
    })();
    const imageScaling = (() => {
      if(this.props.imageSource) {
        return ({
          transform: `scale(${this.props.scaling})`
        });
      } else {
        return ({transform: 'scale(1)'});
      }
    })();
    return (
      <div style={PhotoField.STYLE.wrapper}>
        <div style={boxStyle}>
          <img src={imageSrc}
            style={{...imageStyle, ...imageScaling}}/>
          <div style={cameraIconWrapper}
              onClick={this.props.onToggleUploader}>
            <img src={'resources/account_page/profile_page/photo_field/' +
              'camera.svg'}
              style={cameraIconStyle}/>
          </div>
        </div>
        <Transition
            in={this.props.displayMode === PhotoFieldDisplayMode.UPLOADING}
            timeout={PhotoField.TIMEOUT}>
          {(state) => (
            <div style={{ ...PhotoField.STYLE.animationBase,
                ...(PhotoField.ANIMATION_STYLE as any)[state]}}>
              <ChangePictureModal displaySize={this.props.displaySize}
                imageSource={this.props.newImageSource}
                scaling={this.props.newScaling}
                onCloseModal={this.props.onToggleUploader}
                onSubmitImage={this.props.onSubmit}
                onPhotoChange={this.props.onNewPhotoChange}
                onScalingChange={this.props.onNewScalingChange}/>
            </div>)}
        </Transition>
      </div>);
  }

  private static ANIMATION_STYLE = {
    entering: {
      opacity: 0
    } as React.CSSProperties,
    entered: {
      opacity: 1
    } as React.CSSProperties,
    exited: {
      display: 'none'
    } as React.CSSProperties
  };
  private static readonly STYLE = {
    wrapper: {
      maxHeight: '288px',
      maxWidth: '424px'
    } as React.CSSProperties,
    animationBase: {
      opacity: 0,
      transition: 'opacity 200ms ease'
    } as React.CSSProperties,
    boxSmall: {
      boxSizing: 'border-box',
      backgroundColor: '#F8F8F8',
      width: '100%',
      paddingTop: '68%',
      maxHeight: '288px',
      maxWidth: '424px',
      position: 'relative',
      borderRadius: '1px',
      border: '1px solid #EBEBEB',
      overflow: 'hidden'
    } as React.CSSProperties,
    boxMedium: {
      boxSizing: 'border-box',
      display: 'flex',
      flexDirection: 'row',
      alignItems: 'center',
      justifyContent: 'center',
      backgroundColor: '#F8F8F8',
      border: '1px solid #E6E6E6',
      borderRadius: '1px',
      height: '190px',
      width: '280px',
      position: 'relative',
      overflow: 'hidden'
    } as React.CSSProperties,
    boxLarge: {
      boxSizing: 'border-box',
      display: 'flex',
      flexDirection: 'row',
      alignItems: 'center',
      justifyContent: 'center',
      backgroundColor: '#F8F8F8',
      border: '1px solid #E6E6E6',
      borderRadius: '1px',
      height: '258px',
      width: '380px',
      position: 'relative',
      overflow: 'hidden'
    } as React.CSSProperties,
    placeholder: {
      position: 'absolute',
      height: '24px',
      width: '30px'
    } as React.CSSProperties,
    placeholderSmall: {
      position: 'absolute',
      height: '24px',
      width: '30px',
      top: 'calc(50% - 12px)',
      left: 'calc(50% - 15px)'
    } as React.CSSProperties,
    image: {
      objectFit: 'cover',
      height: '100%',
      width: '100%'
    } as React.CSSProperties,
    imageSmall: {
      position: 'absolute',
      objectFit: 'cover',
      top: '0%',
      left: '0%',
      height: '100%',
      width: '100%'
    } as React.CSSProperties,
    cameraIcon: {
      height: '20px',
      width: '20px'
    } as React.CSSProperties,
    cameraIconWrapper: {
      height: '24px',
      width: '24px',
      display: 'flex',
      justifyContent: 'center',
      alignItems: 'center',
      position: 'absolute',
      top: 'calc(0% + 10px)',
      left: 'calc(100% - 10px - 24px)',
      cursor: 'pointer'
    } as React.CSSProperties,
    hidden: {
      visibility: 'hidden',
      display: 'none'
    } as React.CSSProperties
  };
  private static readonly TIMEOUT = 200;
}

interface ModalProperties {

  /** The image to be displayed. */
  imageSource?: string;

  /** A value that determines how zoomed in the image will be.
   * It is a normalized scalar value.
   */
  scaling: number;

  /** Determines the size at which to display the modal at. */
  displaySize: DisplaySize;

  /** Closes the modal. */
  onCloseModal?: () => void;

  /** Determines what happens when the file is submitted. */
  onSubmitImage?: (newFileLocation: string, scaling: number) => void;

  /** Called when the preview photo changes. */
  onPhotoChange?: (photo: string) => void;

  /** Called to change the slider when the slider moves. */
  onScalingChange?: (scale: number) => void;
}

/** Displays a modal that allows the user to change their picture. */
export class ChangePictureModal extends React.Component<ModalProperties> {
  public static readonly defaultProps = {
    onCloseModal: () => {},
    onSubmitImage: () => {},
    onPhotoChange: () => {},
    onScalingChange: () => {}
  };

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
    const buttonBoxStyle = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return ChangePictureModal.STYLE.buttonBoxSmall;
      } else {
        return ChangePictureModal.STYLE.buttonBoxLarge;
      }
    })();
    const buttonStyle = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return ChangePictureModal.DYNAMIC_STYLE.buttonSmall;
      } else {
        return ChangePictureModal.DYNAMIC_STYLE.buttonLarge;
      }
    })();
    const imageSrc = (() => {
      if(this.props.imageSource) {
        return this.props.imageSource;
      } else {
        return 'resources/account_page/profile_page/photo_field/' +
          'image-placeholder.svg';
      }
    })();
    const imageStyle = (() => {
      if(this.props.imageSource) {
        if(this.props.displaySize === DisplaySize.SMALL) {
          return ChangePictureModal.STYLE.imageSmall;
        } else {
          return ChangePictureModal.STYLE.imageLarge;
        }
      } else {
          return ChangePictureModal.STYLE.placeholderImage;
      }
    })();
    const imageBoxStyle = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return ChangePictureModal.STYLE.imageBoxSmall;
      } else {
        return ChangePictureModal.STYLE.imageBoxLarge;
      }
    })();
    const imageScaling = (() => {
      if(this.props.imageSource) {
        return ({
          transform: `scale(${this.props.scaling})`
        });
      } else {
        return {transform: 'scale(1)'};
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
              <img src={'resources/account_page/profile_page/photo_field/' +
                  'close.svg'} 
                tabIndex={0}
                className={css(ChangePictureModal.DYNAMIC_STYLE.closeIcon)}
                onClick={this.onClose}/>
            </div>
            <Padding size={ChangePictureModal.PADDING_BETWEEN_ELEMENTS}/>
            <div style={imageBoxStyle}>
              <img src={imageSrc}
                style={{...imageStyle, ...imageScaling}}/>
            </div>
            <Padding size={ChangePictureModal.PADDING_BETWEEN_ELEMENTS}/>
            <Slider onChange={this.onSliderMovement}
              displaySize={this.props.displaySize}
              scale={this.props.scaling}
              readonly={!this.props.imageSource}/>
            <Padding size={ChangePictureModal.PADDING_BETWEEN_ELEMENTS}/>
            <HLine color='#E6E6E6' height={1}/>
            <Padding size={ChangePictureModal.PADDING_BETWEEN_ELEMENTS}/>
            <div style={buttonBoxStyle}>
              <input type='file' id='imageInput' accept='image/*'
                style={ChangePictureModal.STYLE.hiddenInput}
                tabIndex={0}
                onChange={(event: React.ChangeEvent<HTMLInputElement>) => {
                  this.onGetImageFile(event.target.files);}}/>
              <label htmlFor='imageInput' tabIndex={0}
                className={css(buttonStyle)}>
                {ChangePictureModal.BROWSE_BUTTON_TEXT}
              </label>
              <div className={css(buttonStyle)}
                onClick={this.onSubmit} tabIndex={0}>
                {ChangePictureModal.SUBMIT_BUTTON_TEXT}
              </div>
            </div>
            <Padding size={ChangePictureModal.PADDING}/>
          </VBoxLayout>
          <Padding size={ChangePictureModal.PADDING}/>
        </HBoxLayout>
      </div>);
  }

  private onSliderMovement = (value: number) => {
    this.props.onScalingChange(value);
  }

  private onGetImageFile = (selectorFiles: FileList) => {
    const file = selectorFiles.item(0);
    const someURL = URL.createObjectURL(file);
    this.props.onPhotoChange(someURL);
    this.setState({});
  }

  private onClose = () => {
    this.props.onCloseModal();
    this.setState({scaling: 1});
  }

  private onSubmit = () => {
    if(this.props.imageSource) {
      this.props.onSubmitImage(this.props.imageSource,
        this.props.scaling);
    }
    this.props.onCloseModal();
  }

  private static readonly STYLE = {
    transparentBackground: {
      boxSizing: 'border-box',
      top: '0px',
      left: '0px',
      position: 'fixed',
      width: '100%',
      height: '100%',
      backgroundColor: '#FFFFFF',
      opacity: 0.9
    } as React.CSSProperties,
    boxShadowSmall:{
      boxSizing: 'border-box',
      opacity: 0.4,
      display: 'block',
      boxShadow: '0px 0px 6px #000000',
      position: 'absolute',
      border: '1px solid #FFFFFF',
      backgroundColor: '#FFFFFF',
      width: '282px',
      height: '100%',
      top: '0%',
      right: '0%'
    } as React.CSSProperties,
    boxShadowLarge:{
      boxSizing: 'border-box',
      opacity: 0.4,
      boxShadow: '0px 0px 6px #000000',
      display: 'block',
      position: 'absolute',
      backgroundColor: '#FFFFFF',
      width: '360px',
      height: '447px',
      top: 'calc(50% - 223.5px)',
      left: 'calc(50% - 180px)'
    } as React.CSSProperties,
    boxSmall: {
      boxSizing: 'border-box',
      display: 'block',
      position: 'absolute',
      border: '1px solid #FFFFFF',
      backgroundColor: '#FFFFFF',
      width: '282px',
      height: '100%',
      top: '0%',
      right: '0%'
    } as React.CSSProperties,
    boxLarge: {
      boxSizing: 'border-box',
      display: 'block',
      position: 'absolute',
      backgroundColor: '#FFFFFF',
      width: '360px',
      height: '447px',
      top: 'calc(50% - 223.5px)',
      left: 'calc(50% - 180px)'
    } as React.CSSProperties,
    header: {
      display: 'flex',
      justifyContent: 'space-between',
      font: '400 16px Roboto'
    } as React.CSSProperties,
    buttonBoxSmall: {
      boxSizing: 'border-box',
      display: 'flex',
      flexDirection: 'column',
      flexWrap: 'wrap',
      alignItems: 'center',
      justifyContent: 'space-between',
      height: '86px'
    } as React.CSSProperties,
    buttonBoxLarge: {
      display: 'flex',
      flexDirection: 'row',
      flexWrap: 'wrap',
      justifyContent: 'space-between',
      alignItems: 'center'
    } as React.CSSProperties,
    placeholderImage: {
      position: 'relative',
      height: '24px',
      width: '30px',
      top: 'calc(50% - 12px)',
      left: 'calc(50% - 15px)'
    } as React.CSSProperties,
    imageSmall: {
      objectFit: 'cover',
      height: '100%',
      width: '100%'
    } as React.CSSProperties,
    imageLarge: {
      objectFit: 'cover',
      height: '100%',
      width: '100%'
    } as React.CSSProperties,
    imageBoxSmall: {
      boxSizing: 'border-box',
      height: '166px',
      width: '246px',
      overflow: 'hidden',
      borderRadius: '1px',
      border: '1px solid #EBEBEB',
      backgroundColor: '#F8F8F8'
    } as React.CSSProperties,
    imageBoxLarge: {
      boxSizing: 'border-box',
      height: '216px',
      width: '324px',
      overflow: 'hidden',
      borderRadius: '1px',
      border: '1px solid #EBEBEB',
      backgroundColor: '#F8F8F8'
    } as React.CSSProperties,
    hiddenInput: {
      width: '0.1px',
      height: '0.1px',
      opacity: 0,
      overflow: 'hidden',
      position: 'absolute'
    } as React.CSSProperties
  };
  private static readonly DYNAMIC_STYLE = StyleSheet.create({
    buttonSmall: {
      boxSizing: 'border-box' as 'border-box',
      cursor: 'pointer' as 'pointer',
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      flexWrap: 'wrap' as 'wrap',
      justifyContent: 'center' as 'center',
      alignItems: 'center' as 'center',
      width: '246px',
      height: '34px',
      backgroundColor: '#684BC7',
      color: '#FFFFFF',
      font: '400 14px Roboto',
      border: '1px solid #684BC7',
      borderRadius: '1px',
      outline: '0px',
      ':active': {
        backgroundColor: '#4B23A0'
      },
      ':hover': {
        backgroundColor: '#4B23A0'
      }
    },
    buttonLarge: {
      boxSizing: 'border-box' as 'border-box',
      cursor: 'pointer' as 'pointer',
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      flexWrap: 'wrap' as 'wrap',
      justifyContent: 'center' as 'center',
      alignItems: 'center' as 'center',
      width: '153px',
      height: '34px',
      backgroundColor: '#684BC7',
      color: '#FFFFFF',
      font: '400 14px Roboto',
      border: '1px solid #684BC7',
      borderRadius: '1px',
      outline: '0px',
      ':active': {
        backgroundColor: '#4B23A0'
      },
      ':hover': {
        backgroundColor: '#4B23A0'
      }
    },
    closeIcon: {
      width: '20px',
      height: '20px',
      cursor: 'pointer' as 'pointer',
      ':focus': {
        outline: 0
      },
      ':active': {
        outline: 0,
        border: 0
      },
      '::moz-focus-inner': {
        border: 0
      }
    }
  });
  private static readonly HEADER_TEXT = 'Change Picture';
  private static readonly BROWSE_BUTTON_TEXT = 'Browse';
  private static readonly SUBMIT_BUTTON_TEXT = 'Submit';
  private static readonly PADDING = '18px';
  private static readonly PADDING_BETWEEN_ELEMENTS = '30px';
}

interface SliderProperties {

  /** Determines the size to render the component at. */
  displaySize: DisplaySize;

  /** Callback that updates the value */
  onChange?: (value: number) => void;

  /** Determines if the slider can be moved. */
  readonly?: boolean;

  /** The current slider value. */
  scale?: number;
}

/** Displays a slider that changes a value. */
export class Slider extends React.Component<SliderProperties, {}> {
  public static readonly defaultProps = {
    onChange: () => {},
    scale: 0,
    readonly: false
  };

  public render(): JSX.Element {
    const sliderStyle = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return Slider.SLIDER_STYLE.smallSlider;
      } else {
        return Slider.SLIDER_STYLE.slider;
      }
    })();
    const cursorStyle = (() => {
      if(this.props.readonly) {
        return Slider.SLIDER_STYLE.disabled;
      } else {
        return Slider.SLIDER_STYLE.enabled;
      }
    })();
    return (<input type='range'
      min={Slider.MIN_RANGE_VALUE}
      max={Slider.MAX_RANGE_VALUE}
      value={Slider.convertFromDecimal(this.props.scale)}
      disabled={this.props.readonly}
      onChange={this.onValueChange}
      className={css(sliderStyle, cursorStyle)}/>);
  }

  private onValueChange = (event: any) => {
    const num = event.target.value;
    const diff = Math.abs(this.props.scale - num);
    if(this.props.scale < num) {
      this.props.onChange(Slider.convertToDecimal(
          this.props.scale + diff));
    } else {
      this.props.onChange(Slider.convertToDecimal(
          this.props.scale - diff));
    }
  }

  private static convertToDecimal(value: number) {
    return (100 + value) / 100;
  }

  private static convertFromDecimal(value: number) {
    return (value * 100) - 100;
  }

  public static readonly SLIDER_STYLE = StyleSheet.create({
    slider: {
      width: '100%',
      height: '20px',
      margin: '0px',
      outline: '0px',
      ':disabled' : {
        backgroundColor: '#FFFFFF'
      },
      '::-webkit-slider-thumb': {
        '-webkit-appearance': 'none',
        boxSizing: 'border-box' as 'border-box',
        height: '20px',
        width: '20px',
        backgroundColor: '#FFFFFF',
        border: '6px solid #684BC7',
        borderRadius: '20px',
        boxShadow: 'none',
        marginTop: '-8px'
      },
      '::-moz-range-thumb': {
        boxSizing: 'border-box' as 'border-box',
        height: '20px',
        width: '20px',
        backgroundColor: '#FFFFFF',
        border: '6px solid #684BC7',
        borderRadius: '20px'
      },
      '::-ms-thumb': {
        boxSizing: 'border-box' as 'border-box',
        height: '20px',
        width: '20px',
        backgroundColor: '#FFFFFF',
        border: '6px solid #684BC7',
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
        border: '0px'
      },
      '::-ms-track': {
        backgroundColor: '#E6E6E6',
        height: '4px'
      },
      '-webkit-appearance': 'none',
      '::-moz-focus-outer': {
        border: '0px'
      }
    },
    smallSlider: {
      width: '100%',
      height: '24px',
      margin: '0px',
      outline: '0px',
      ':disabled' : {
        backgroundColor: '#FFFFFF'
      },
      '::-webkit-slider-thumb': {
        '-webkit-appearance': 'none',
        boxSizing: 'border-box' as 'border-box',
        height: '24px',
        width: '24px',
        backgroundColor: '#FFFFFF',
        border: '6px solid #684BC7',
        borderRadius: '24px',
        boxShadow: 'none',
        marginTop: '-8px'
      },
      '::-moz-range-thumb': {
        boxSizing: 'border-box' as 'border-box',
        height: '24px',
        width: '24px',
        backgroundColor: '#FFFFFF',
        border: '6px solid #684BC7',
        borderRadius: '24px'
      },
      '::-ms-thumb': {
        boxSizing: 'border-box' as 'border-box',
        height: '24px',
        width: '24px',
        backgroundColor: '#FFFFFF',
        border: '6px solid #684BC7',
        borderRadius: '24px',
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
        border: '0px'
      },
      '::-ms-track': {
        backgroundColor: '#E6E6E6',
        height: '4px'
      },
      '-webkit-appearance': 'none',
      '::-moz-focus-outer': {
        border: '0px'
      }
    },
    enabled:{
      '::-webkit-slider-thumb': {
        cursor: 'pointer' as 'pointer'
      },
      '::-moz-range-thumb': {
        cursor: 'pointer' as 'pointer'
      },
      '::-ms-thumb': {
        cursor: 'pointer' as 'pointer'
      }
    },
    disabled: {
      '::-webkit-slider-thumb': {
        cursor: 'default' as 'default'
      },
      '::-moz-range-thumb': {
        cursor: 'default' as 'default'
      },
      '::-ms-thumb': {
        cursor: 'default' as 'default'
      }
    }
  });
  private static readonly MIN_RANGE_VALUE = 0;
  private static readonly MAX_RANGE_VALUE = 200;
}
