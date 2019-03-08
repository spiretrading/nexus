import { css, StyleSheet } from 'aphrodite';
import * as Beam from 'beam';
import * as React from 'react';
import { DisplaySize } from '../../display_size';
import { element } from 'prop-types';

interface Properties {
  displaySize: DisplaySize;
  value?: string;
  selectedGroups?: Beam.DirectoryEntry[];
  suggestions?: Beam.DirectoryEntry[];
  isError?: boolean;
  onValueChange?: (newValue: string) => void;
  addGroup?: (group: Beam.DirectoryEntry) => void;
  removeGroup?: (group: Beam.DirectoryEntry) => void;
}

interface State {
  currentIndex: number;
  showSuggestions: boolean;
}

export class GroupSelectionBox extends React.Component<Properties, State> {
  public static readonly defaultProps = {
    selectedGroups:  new Array<Beam.DirectoryEntry>(),
    suggestions: new Array<Beam.DirectoryEntry>(),
    isError: false,
    addGroup: () => {},
    removeGroup: () => {},
    onValue: () => {}
  };

  constructor(props: Properties) {
    super(props);
    this.state = {
      currentIndex : 0,
      showSuggestions: false
    };
    this.changeIndex = this.changeIndex.bind(this);
    this.addGroup = this.addGroup.bind(this);
    this.onKeyPress = this.onKeyPress.bind(this);
    this.inputRef = null;
  }

  public render(): JSX.Element {
    const boxStyle = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return GroupSelectionBox.DYNAMIC_STYLE.boxSmall;
      } else if(this.props.displaySize === DisplaySize.MEDIUM) {
        return GroupSelectionBox.DYNAMIC_STYLE.boxMedium;
      } else {
        return GroupSelectionBox.DYNAMIC_STYLE.boxLarge;
      }
    })();
    const inputStyle = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return GroupSelectionBox.DYNAMIC_STYLE.inputSmall;
      } else if(this.props.displaySize === DisplaySize.MEDIUM) {
        return GroupSelectionBox.DYNAMIC_STYLE.inputMedium;
      } else {
        return GroupSelectionBox.DYNAMIC_STYLE.inputLarge;
      }
    })();
    const errorBoxStyle = (() => {
      if(this.props.isError) {
        return GroupSelectionBox.DYNAMIC_STYLE.error;
      } else {
        return null;
      }
    })();
    const textStyle = (() => {
      if (this.props.displaySize === DisplaySize.SMALL) {
        return GroupSelectionBox.DYNAMIC_STYLE.textSmall;
      } else {
        return GroupSelectionBox.DYNAMIC_STYLE.textLarge;
      }
    })();
    const suggestionBox = (() => {
      if(document.activeElement === this.inputRef
        && this.props.selectedGroups) {
        return (
        <SuggestionBox
          currentIndex={this.state.currentIndex}
          suggestedGroups={this.props.suggestions}
          displaySize={this.props.displaySize}
          addGroup={this.addGroup}
          changeIndex={this.changeIndex}/>);
      } else {
        return null;
      }
    })();
    const selectedGroups = [];
      for(const group of this.props.selectedGroups) {
        selectedGroups.push(
          <div className={css(GroupSelectionBox.DYNAMIC_STYLE.groupEntry)}
              key={group.id}>
            <div className={css(GroupSelectionBox.DYNAMIC_STYLE.textLarge)}>
              {group.name}
            </div>
            <div className={css(GroupSelectionBox.DYNAMIC_STYLE.imageWrapper)}>
              <img className={css(GroupSelectionBox.DYNAMIC_STYLE.image)}
                onClick={ () => this.props.removeGroup(group) }
                src={'resources/remove.svg'}/>
            </div>
          </div>);
  }
    return (
      <div id='GROUP BOX' className={css(boxStyle)}>
        <input type='text'
          value={this.props.value}
          onKeyDown={(event: React.KeyboardEvent<HTMLInputElement>) =>
            this.onKeyPress(event)}
          ref={ (e) => {this.inputRef = e;} }
          onChange={(event: React.ChangeEvent<HTMLInputElement>) => {
            this.props.onValueChange(event.target.value);}}
          className={css(inputStyle, errorBoxStyle, textStyle)}/>
          <div className={css(GroupSelectionBox.DYNAMIC_STYLE.filler)}/>
            <div
             className={css(GroupSelectionBox.DYNAMIC_STYLE.suggestionWrapper)}>
            {suggestionBox}
          </div>
          {selectedGroups}
      </div>);
  }
  private addGroup() {
    if (this.props.suggestions) {
      if(this.props.suggestions.length > this.state.currentIndex &&
          this.state.currentIndex >= 0) {
        const thing =  this.props.suggestions[this.state.currentIndex];
        if(thing) {
          this.props.addGroup(thing);
        }
      }
    }
  }

  private onKeyPress(event: React.KeyboardEvent<HTMLInputElement>) {
    console.log('current keycodeeeee: ' + event.keyCode );
    if(event.keyCode === 38) {
      if(this.state.currentIndex > 0) {
        this.setState({ currentIndex: this.state.currentIndex - 1 });
      }
    } else if (event.keyCode === 40) {
      if(this.state.currentIndex < this.props.suggestions.length - 1) {
        this.setState({ currentIndex: this.state.currentIndex + 1 });
      }
    } else if (event.keyCode === 13 && this.props.suggestions
      && this.state.currentIndex >= 0) {
      this.addGroup();
    }
  }

  private changeIndex(newCurrent?: number) {
    this.setState({ currentIndex: newCurrent });
  }

  public componentDidUpdate(prevProps: Properties) {
    if(this.props.suggestions === null && this.state.currentIndex !== -1) {
      this.setState({ currentIndex: -1 });
    } else if(this.props.suggestions !== prevProps.suggestions) {
      this.setState({ currentIndex: 0 });
    }

  }
  private static DYNAMIC_STYLE = StyleSheet.create({
    boxSmall : {
      display: 'flex',
      flexDirection: 'column',
      justifyContent: 'flex-start',
      alignItems: 'center',
      width: '100%'
    },
    boxMedium : {
      display: 'flex',
      flexDirection: 'column',
      justifyContent: 'flex-start',
      alignItems: 'center',
      width: '100%'
    },
    boxLarge : {
      display: 'flex',
      flexDirection: 'column',
      justifyContent: 'flex-start',
      alignItems: 'center',
      width: '100%'
    },
    inputSmall: {
      boxSizing: 'border-box' as 'border-box',
      paddingLeft: '10px',
      height: '34px',
      flexGrow: 0,
      flexShrink: 0,
      width: '100%',
      border: '1px solid #C8C8C8',
      borderRadius: '1px',
      ':focus': {
        borderColor: '#684BC7',
        ouline: 'none',
        boxShadow: 'none',
        webkitBoxShadow: 'none',
        outlineColor: 'transparent',
        outlineStyle: 'none'
      },
      ':active': {
        borderColor: '#684BC7'
      },
      '::moz-focus-inner': {
        border: 0
      }
    },
    inputMedium: {
      boxSizing: 'border-box' as 'border-box',
      paddingLeft: '10px',
      height: '34px',
      flexGrow: 0,
      flexShrink: 0,
      width: '284px',
      border: '1px solid #C8C8C8',
      borderRadius: '1px',
      ':focus': {
        borderColor: '#684BC7',
        ouline: 'none',
        boxShadow: 'none',
        webkitBoxShadow: 'none',
        outlineColor: 'transparent',
        outlineStyle: 'none'
      },
      ':active': {
        borderColor: '#684BC7'
      },
      '::moz-focus-inner': {
        border: 0
      }
    },
    inputLarge: {
      boxSizing: 'border-box' as 'border-box',
      paddingLeft: '10px',
      height: '34px',
      width: '350px',
      flexGrow: 0,
      flexShrink: 0,
      border: '1px solid #C8C8C8',
      borderRadius: '1px',
      ':focus': {
        borderColor: '#684BC7',
        ouline: 'none',
        boxShadow: 'none',
        webkitBoxShadow: 'none',
        outlineColor: 'transparent',
        outlineStyle: 'none'
      },
      ':active': {
        borderColor: '#684BC7'
      },
      '::moz-focus-inner': {
        border: 0
      }
    },
    groupEntry : {
      boxSizing: 'border-box' as 'border-box',
      paddingLeft: '10px',
      display: 'flex',
      flexDirection: 'row',
      flexWrap: 'nowrap',
      justifyContent: 'space-between',
      alignItems: 'center',
      height: '34px',
      width: '100%'
    },
    imageWrapper: {
      paddingLeft:'10px',
      paddingRight: '5px',
      display: 'flex',
      flexDirection: 'row',
      flexWrap: 'nowrap',
      justifyContent: 'center',
      height: '34px',
      alignItems: 'center'
    },
    image: {
      width: '12px',
      height: '12px',
      padding: '6px'
    },
    textLarge: {
      font: '400 14px Roboto',
      color: '#000000'
    },
    textSmall: {
      font: '400 16px Roboto',
      color: '#000000'
    },
    error: {
      border: '1px solid #E63F44'
    },
    filler: {
      width: '100%',
      height: '5px'
    },
    suggestionWrapper: {
      position: 'relative' as 'relative',
      height: '1px',
      width: '100%',
      tabIndex: -1
    }
  });
  private inputRef: HTMLInputElement;
}

interface SuggestionBoxProps {
  displaySize: DisplaySize;
  suggestedGroups?: Beam.DirectoryEntry[];
  currentIndex: number;
  changeIndex?: (newIndex: number) => void;
  addGroup?: () => void;
}

interface SuggestionBoxState {
  currentIndex?: number;
}

class SuggestionBox extends React.Component<SuggestionBoxProps> {
  public static readonly defaultProps = {
    changeIndex: () => {},
    addGroup: () => {}
  };

  constructor(props: SuggestionBoxProps) {
    super(props);
    this.currentEntryRef = React.createRef();
    this.unorderedListRef = React.createRef();
  }

  public render(): JSX.Element {
    const textStyle = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return SuggestionBox.DYNAMIC_STYLE.textSmall;
      }  else {
        return SuggestionBox.DYNAMIC_STYLE.textLarge;
      }
    })();
    const selectedGroups = [];
    if(this.props.suggestedGroups !== null) {
      for(let index = 0; index < this.props.suggestedGroups.length; ++index) {
        const group = this.props.suggestedGroups[index];
          if(this.props.currentIndex === index) {
            selectedGroups.push(
        <li className={css(SuggestionBox.DYNAMIC_STYLE.entry,
              textStyle, SuggestionBox.DYNAMIC_STYLE.selected)}
            onMouseEnter={() => this.props.changeIndex(index)}
            onClick = { () => this.props.addGroup()}
            key={index}
            ref={this.currentEntryRef}
            id={index.toString()}>
          {group.name}
        </li>);
          } else {
            selectedGroups.push(
        <li className={css(SuggestionBox.DYNAMIC_STYLE.entry, textStyle)}
            onMouseEnter={() => this.props.changeIndex(index)}
            onClick = { () => this.props.addGroup()}
            key={index}
            id={index.toString()}>
          {group.name}
        </li>);
          }
      }
    }
    return (
        <ul className={css(SuggestionBox.DYNAMIC_STYLE.box)}
          tabIndex={-1}
          ref={this.unorderedListRef}>
         {selectedGroups}
        </ul>
    );
 }

  public componentDidUpdate() {
    if(this.props.suggestedGroups !== null && this.props.suggestedGroups !== []) {
      if(this.currentEntryRef.current !== null) {
        const child = this.currentEntryRef.current;
          const childBound =
            child.getBoundingClientRect();
            console.log('wut');
          const boundPar =
            child.getBoundingClientRect();
          if( childBound.top < boundPar.top) {
            this.currentEntryRef.current.scrollIntoView();
          } else if( childBound.bottom > boundPar.bottom) {
            this.currentEntryRef.current.scrollIntoView(false);
          }
      }
    }
  }

  private static DYNAMIC_STYLE = StyleSheet.create({
    entry: {
      zIndex: 1,
      height: '34px',
      paddingLeft: '10px',
      font: '400 14px Roboto',
      color: '#000000',
      display: 'flex',
      justifyContent: 'flex-start',
      alignItems: 'center',
      cursor: 'pointer as pointer'
    },
    textSmall: {
      font: '400 16px Roboto',
      color: '#000000'
    },
    textLarge: {
      font: '400 14px Roboto',
      color: '#000000'
    },
    selected: {
      color: '#FFFFFF',
      backgroundColor: '#684BC7'
    },
    box: {
      boxSizing: 'border-box' as 'border-box',
      zIndex: 1,
      maxHeight: '136px',
      overflow: 'auto' as 'auto',
      backgroundColor: '#FFFFFF',
      boxShadow: '0px 2px 6px #C8C8C8',
      width: '100%',
      position: 'absolute' as 'absolute',
      padding: '0px',
      margin: '0px',
      tabFocus: -1
    }
  });
  private currentEntryRef: React.RefObject<HTMLLIElement>;
  private unorderedListRef: React.RefObject<HTMLUListElement>;
}
