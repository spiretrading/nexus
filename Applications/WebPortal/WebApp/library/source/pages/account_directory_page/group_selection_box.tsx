import { css, StyleSheet } from 'aphrodite';
import * as Beam from 'beam';
import * as React from 'react';
import { DisplaySize } from '../../display_size';

interface Properties {

  /** The size to display the element at. */
  displaySize: DisplaySize;

  /** The value inside the input box. */
  value?: string;

  /** The list of groups that are selected. */
  selectedGroups?: Beam.DirectoryEntry[];

  /** The list of suggestions. */
  suggestions?: Beam.DirectoryEntry[];

  /** Determines if error mode should be displayed. */
  isError?: boolean;

  /** The value inside the input box.
   * @param newValue - The new value.
   */
  onValueChange?: (newValue: string) => void;

  /** Add a group to the selected groups.
   * @param group - The group to be added.
   */
  addGroup?: (group: Beam.DirectoryEntry) => void;

  /** Remove a group from the selected groups.
   * @param group - The new value.
   */
  removeGroup?: (group: Beam.DirectoryEntry) => void;
}

interface State {
  currentIndex: number;
  showSuggestions: boolean;
}

/** Displays a box that shows suggested groups based on the input string. */
export class GroupSelectionBox extends React.Component<Properties, State> {
  public static readonly defaultProps = {
    value: '',
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
    this.inputRef = React.createRef();
    this.addGroup = this.addGroup.bind(this);
    this.changeIndex = this.changeIndex.bind(this);
    this.onKeyPress = this.onKeyPress.bind(this);
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
      if(this.state.showSuggestions && this.props.suggestions !== null) {
        return (
        <div className={css(GroupSelectionBox.DYNAMIC_STYLE.suggestionWrapper)}>
          <SuggestionBox
            currentIndex={this.state.currentIndex}
            suggestedGroups={this.props.suggestions}
            displaySize={this.props.displaySize}
            addGroup={this.addGroup}
            changeIndex={this.changeIndex}/>
            </div>);
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
    <div id='GROUP BOX' className={css(boxStyle)}
        onFocus={() => this.setState({showSuggestions: true })}
        onBlur={() => this.setState({showSuggestions: false })}
        >
      <input type='text'
        ref={this.inputRef}
        value={this.props.value}
        onKeyDown={(event: React.KeyboardEvent<HTMLInputElement>) =>
          this.onKeyPress(event)}
        onChange={(event: React.ChangeEvent<HTMLInputElement>) => {
          this.props.onValueChange(event.target.value);}}
        className={css(inputStyle, errorBoxStyle, textStyle)}/>
      <div className={css(GroupSelectionBox.DYNAMIC_STYLE.filler)}/>
      {suggestionBox}
      {selectedGroups}
    </div>);
  }

  public componentDidUpdate(prevProps: Properties) {
    if(this.props.suggestions === null && this.state.currentIndex !== -1) {
      this.setState({
        currentIndex: -1
      });
    } else if(this.props.suggestions !== prevProps.suggestions) {
      this.setState({
        currentIndex: 0
      });
    }
  }

  private addGroup() {
    console.log('add the thing!');
    if (this.props.suggestions !== null && this.state.currentIndex >= 0) {
        const thing =  this.props.suggestions[this.state.currentIndex];
        if(thing) {
          this.props.addGroup(thing);
        }
        this.inputRef.current.focus();
      }
  }

  private changeIndex(newCurrent?: number) {
    this.setState({ currentIndex: newCurrent });
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
      width: '100%',
      ':hover' : {
        backgroundColor: '#F8F8F8'
      },
      ':active' : {
        backgroundColor: '#F8F8F8'
      }
    },
    imageWrapper: {
      paddingLeft:'10px',
      paddingRight: '5px',
      display: 'flex',
      flexDirection: 'row',
      flexWrap: 'nowrap',
      justifyContent: 'center',
      height: '34px',
      alignItems: 'center',
      cursor: 'pointer' as 'pointer'
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
  private inputRef: React.RefObject<HTMLInputElement>;
}

interface SuggestionBoxProps {
  displaySize: DisplaySize;
  currentIndex: number;
  suggestedGroups?: Beam.DirectoryEntry[];
  changeIndex?: (newIndex: number) => void;
  addGroup?: () => void;
}

class SuggestionBox extends React.Component<SuggestionBoxProps> {
  public static readonly defaultProps = {
    suggestedGroups: new Array<Beam.DirectoryEntry>(),
    changeIndex: () => {},
    addGroup: () => {}
  };

  constructor(props: SuggestionBoxProps) {
    super(props);
    this.currentEntryRef = React.createRef();
  }

  public render(): JSX.Element {
    const textStyle = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return SuggestionBox.STYLE.textSmall;
      } else {
        return SuggestionBox.STYLE.textLarge;
      }
    })();
    const selectedGroups = [];
    if(this.props.suggestedGroups !== null) {
      for(let index = 0; index < this.props.suggestedGroups.length; ++index) {
        const group = this.props.suggestedGroups[index];
        if(this.props.currentIndex === index) {
          selectedGroups.push(
            <li style={{...SuggestionBox.STYLE.entry,
                  ...textStyle, ...SuggestionBox.STYLE.selected}}
                onClick={this.props.addGroup}
                key={index}
                ref={this.currentEntryRef}>
              {group.name}
            </li>);
        } else {
          selectedGroups.push(
            <li style={{...SuggestionBox.STYLE.entry, ...textStyle}}
                onMouseMove={() => this.props.changeIndex(index)}
                onClick={this.props.addGroup}
                key={index}>
              {group.name}
            </li>);
        }
      }
    }
    return (
      <ul style={SuggestionBox.STYLE.box}
          tabIndex={-1}>
        {selectedGroups}
      </ul>
    );
 }

  public componentDidUpdate() {
    if(this.currentEntryRef.current !== null) {
      const childBounding =
        this.currentEntryRef.current.getBoundingClientRect();
      const parentBounding =
        this.currentEntryRef.current.parentElement.getBoundingClientRect();
      if( childBounding.top < parentBounding.top) {
        this.currentEntryRef.current.scrollIntoView();
      } else if( childBounding.bottom > parentBounding.bottom) {
        this.currentEntryRef.current.scrollIntoView(false);
      }
    }
  }

  private static STYLE = {
    entry: {
      zIndex: 1,
      height: '34px',
      paddingLeft: '10px',
      font: '400 14px Roboto',
      color: '#000000',
      display: 'flex',
      justifyContent: 'flex-start',
      alignItems: 'center',
      cursor: 'default',
      tabIndex: -1
    },
    textSmall: {
      font: '400 16px Roboto',
      color: '#000000',
      cursor: 'default'
    },
    textLarge: {
      font: '400 14px Roboto',
      color: '#000000',
      cursor: 'default'
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
  };
  private currentEntryRef: React.RefObject<HTMLLIElement>;
}
