import { css, StyleSheet } from 'aphrodite';
import * as Beam from 'beam';
import * as React from 'react';
import { DisplaySize } from '../../..';

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

  /** Called to update the value inside the input box.
   * @param newValue - The new value.
   */
  onValueChange?: (newValue: string) => void;

  /** Called to add a group to the selected groups.
   * @param group - The group to be added.
   */
  onAddGroup?: (group: Beam.DirectoryEntry) => void;

  /** Called to remove signal a group needs to be removed.
   * @param group - The new value.
   */
  onRemoveGroup?: (group: Beam.DirectoryEntry) => void;
}

interface State {
  currentIndex: number;
  showSuggestions: boolean;
}

/** Displays a box that shows suggested groups based on the input string. */
export class GroupSelectionBox extends React.Component<Properties, State> {
  public static readonly defaultProps = {
    value: '',
    selectedGroups: [] as Beam.DirectoryEntry[],
    suggestions: [] as Beam.DirectoryEntry[],
    isError: false,
    onAddGroup: () => {},
    onRemoveGroup: () => {},
    onValueChange: () => {}
  };

  constructor(props: Properties) {
    super(props);
    this.state = {
      currentIndex : 0,
      showSuggestions: false
    };
    this.inputRef = React.createRef();
    this.onValueChange = this.onValueChange.bind(this);
    this.onFocus = this.onFocus.bind(this);
    this.onBlur = this.onBlur.bind(this);
    this.onAddGroup = this.onAddGroup.bind(this);
    this.onChangeIndex = this.onChangeIndex.bind(this);
    this.onKeyPress = this.onKeyPress.bind(this);
  }

  public render(): JSX.Element {
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
      if(this.props.displaySize === DisplaySize.SMALL) {
        return GroupSelectionBox.DYNAMIC_STYLE.textSmall;
      } else {
        return GroupSelectionBox.DYNAMIC_STYLE.textLarge;
      }
    })();
    const suggestionBox = (() => {
      if(this.state.showSuggestions) {
        return GroupSelectionBox.STYLE.suggestionWrapper;
      } else {
        return GroupSelectionBox.STYLE.hidden;
      }
    })();
    const selectedGroups = this.props.selectedGroups.map((group) => {
      return (<div className={css(GroupSelectionBox.DYNAMIC_STYLE.groupEntry)}
          key={group.id}>
        <div className={css(textStyle)}>
          {group.name}
        </div>
        <div style={GroupSelectionBox.STYLE.imageWrapper}>
          <img style={GroupSelectionBox.STYLE.image}
            onClick={ () => this.props.onRemoveGroup(group) }
            src='resources/remove.svg'/>
        </div>
      </div>);
    });
  return (
    <div style={GroupSelectionBox.STYLE.box}
        onBlur={this.onBlur}
        onFocus={this.onFocus}>
      <input type='text'
        ref={this.inputRef}
        value={this.props.value}
        onKeyDown={this.onKeyPress}
        onChange={this.onValueChange}
        className={css(inputStyle, textStyle, errorBoxStyle)}/>
      <div style={GroupSelectionBox.STYLE.filler}/>
      <div style={suggestionBox}>
        <SuggestionBox
          currentIndex={this.state.currentIndex}
          suggestedGroups={this.props.suggestions}
          displaySize={this.props.displaySize}
          onAddGroup={this.onAddGroup}
          onChangeIndex={this.onChangeIndex}/>
      </div>
      {selectedGroups}
    </div>);
  }

  public componentDidUpdate(prevProps: Properties): void {
    if(this.props.suggestions === [] && this.state.currentIndex !== -1) {
      this.setState({
        currentIndex: -1
      });
    } else if(this.props.suggestions !== prevProps.suggestions) {
      this.setState({
        currentIndex: 0
      });
    }
  }

  private onValueChange(event: React.ChangeEvent<HTMLInputElement>) {
    this.props.onValueChange(event.target.value);
  }

  private onFocus() {
    this.setState({showSuggestions: true});
  }

  private onBlur() {
    this.setState({showSuggestions: false});
  }

  private onAddGroup() {
    if(this.props.suggestions !== [] && this.state.currentIndex >= 0) {
      const newGroup = this.props.suggestions[this.state.currentIndex];
      if(newGroup) {
        this.props.onAddGroup(newGroup);
      }
    }
    this.inputRef.current.focus();
  }

  private onChangeIndex(newCurrent: number) {
    this.setState({ currentIndex: newCurrent });
  }

  private onKeyPress(event: React.KeyboardEvent<HTMLInputElement>) {
    if(event.keyCode === 38) {
      if(this.state.currentIndex > 0) {
        this.setState({ currentIndex: this.state.currentIndex - 1 });
      }
    } else if(event.keyCode === 40) {
      if(this.state.currentIndex < this.props.suggestions.length - 1) {
        this.setState({ currentIndex: this.state.currentIndex + 1 });
      }
    } else if(event.keyCode === 13 && this.props.suggestions &&
        this.state.currentIndex >= 0) {
      this.onAddGroup();
    }
  }

  private static STYLE = {
    box : {
      display: 'flex' as 'flex',
      flexDirection: 'column' as 'column',
      justifyContent: 'flex-start' as 'flex-start',
      alignItems: 'center' as 'center',
      width: '100%'
    },
    imageWrapper: {
      paddingLeft:'10px',
      paddingRight: '5px',
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      flexWrap: 'nowrap' as 'nowrap',
      justifyContent: 'center' as 'center',
      height: '34px',
      alignItems: 'center' as 'center',
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
      height: '4px'
    },
    suggestionWrapper: {
      position: 'relative' as 'relative',
      height: '1px',
      width: '100%',
      tabIndex: -1
    },
    hidden :{
      height: '1px',
      visibility: 'hidden' as 'hidden'
    }
  };

  private static DYNAMIC_STYLE = StyleSheet.create({
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
    }
  });
  private inputRef: React.RefObject<HTMLInputElement>;
}

interface SuggestionBoxProps {

  /** The size the element is displayed at. */
  displaySize: DisplaySize;

  /** The index of the current selected suggestion. */
  currentIndex: number;

  /** The list of suggested groups. */
  suggestedGroups?: Beam.DirectoryEntry[];

  /** Changes the currentIndex.
   * @param newIndex - The value the index should be changed to.
   */
  onChangeIndex?: (newIndex: number) => void;

  /** Adds the group at the CurrentIndex to the selected groups. */
  onAddGroup?: () => void;
}

class SuggestionBox extends React.Component<SuggestionBoxProps> {
  public static readonly defaultProps = {
    suggestedGroups: [] as Beam.DirectoryEntry[],
    onChangeIndex: () => {},
    onAddGroup: () => {}
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
    const suggestions = this.props.suggestedGroups.map((group, index) => {
      if(group === this.props.suggestedGroups[this.props.currentIndex]) {
        return (
          <li style={{...SuggestionBox.STYLE.entry,
                ...textStyle, ...SuggestionBox.STYLE.selected}}
              onMouseDown={this.props.onAddGroup}
              key={group.id}
              ref={this.currentEntryRef}>
            {group.name}
          </li>);
      } else {
        return (
          <li style={{...SuggestionBox.STYLE.entry, ...textStyle}}
              onMouseMove={() => this.props.onChangeIndex(index)}
              key={group.id}>
            {group.name}
          </li>);
      }
    });
    return (
      <ul className={css(SuggestionBox.DYNAMIC_STYLE.box)}
          tabIndex={-1}>
        {suggestions}
      </ul>);
 }

  public componentDidUpdate(): void {
    if(this.currentEntryRef.current !== null) {
      const childBounding =
        this.currentEntryRef.current.getBoundingClientRect();
      const parentBounding =
        this.currentEntryRef.current.parentElement.getBoundingClientRect();
      if(childBounding.top < parentBounding.top) {
        this.currentEntryRef.current.scrollIntoView();
      } else if(childBounding.bottom > parentBounding.bottom) {
        this.currentEntryRef.current.scrollIntoView(false);
      }
    }
  }

  private static STYLE = {
    entry: {
      zIndex: 1,
      height: '34px',
      paddingLeft: '10px',
      color: '#000000',
      display: 'flex',
      justifyContent: 'flex-start',
      alignItems: 'center',
      cursor: 'default'
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
    }
  };

  private static DYNAMIC_STYLE = StyleSheet.create({
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
      tabFocus: -1,
     ':focus': {
        ouline: 'none',
        borderStyle: 'none',
        boxShadow: '0px 2px 6px #C8C8C8',
        borderColor: 'transparent',
        webkitBoxShadow: 'none',
        outlineColor: 'transparent',
        outlineStyle: 'none'
      },
      ':active': {
        ouline: 'none',
        borderStyle: 'none',
        boxShadow: '0px 2px 6px #C8C8C8',
        borderColor: 'transparent',
        webkitBoxShadow: 'none',
        outlineColor: 'transparent',
        outlineStyle: 'none'
      },
      '::-moz-focus-inner': {
        ouline: 'none',
        borderStyle: 'none',
        boxShadow: '0px 2px 6px #C8C8C8',
        borderColor: 'transparent',
        webkitBoxShadow: 'none',
        outlineColor: 'transparent',
        outlineStyle: 'none'
      }
    }
  });

  private currentEntryRef: React.RefObject<HTMLLIElement>;
}
