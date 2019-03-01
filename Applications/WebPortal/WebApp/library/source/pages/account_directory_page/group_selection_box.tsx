import { css, StyleSheet } from 'aphrodite';
import * as Beam from 'beam';
import * as React from 'react';
import { DisplaySize } from '../../display_size';
import { GroupSuggestionModel } from '.';

interface Properties {
  displaySize: DisplaySize;
  error?: boolean;
  value?: string;
  selectedGroups?: Beam.Set<Beam.DirectoryEntry>;
  suggestions?: Beam.Set<Beam.DirectoryEntry>;

  onValueChange?: (newValue: string) => void;
  addGroup?: (group: Beam.DirectoryEntry) => void;
  removeGroup?: (group: Beam.DirectoryEntry) => void;
}

export class GroupSelectionBox extends React.Component<Properties> {
  public static readonly defaultProps = {
    selectedGroups:  new Beam.Set<Beam.DirectoryEntry>(),
    suggestions:  new Beam.Set<Beam.DirectoryEntry>(),
    addGroup: () => {},
    removeGroup: () => {}
  };

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
      if(this.props.error) {
        return GroupSelectionBox.DYNAMIC_STYLE.error;
      } else {
        return null;
      }
    })();
    const selectedGroups = [];
    for(const group of this.props.selectedGroups) {
      selectedGroups.push(
      <div className={css(GroupSelectionBox.DYNAMIC_STYLE.groupEntry)}
        key={group.id}>
        <div className={css(GroupSelectionBox.DYNAMIC_STYLE.text)}>
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
          onChange={(event: React.ChangeEvent<HTMLInputElement>) => {
            this.props.onValueChange(event.target.value);
          }}
          className={css(inputStyle, errorBoxStyle,
          GroupSelectionBox.DYNAMIC_STYLE.text)}/>
          <div className={css(GroupSelectionBox.DYNAMIC_STYLE.filler)}/>
                    <div
            className={css(GroupSelectionBox.DYNAMIC_STYLE.suggestionWrapper)}>
            <SuggestionBox
              suggestedGroups={this.props.suggestions}
              displaySize={this.props.displaySize}
              addGroup={this.props.addGroup}/>
          </div>
          {selectedGroups}
      </div>);
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
    text: {
      font: '400 14px Roboto',
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
        width: '100%'
    }
  });
}

interface SuggestionBoxProps {
  suggestedGroups?: Beam.Set<Beam.DirectoryEntry>;
  displaySize: DisplaySize;
  addGroup?: (group: Beam.DirectoryEntry) => void;
}

class SuggestionBox extends React.Component<SuggestionBoxProps> {
  public static readonly defaultProps = {
    addGroup: () => {}
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
    for(const group of this.props.suggestedGroups) {
      selectedGroups.push(
      <div className={css(SuggestionBox.DYNAMIC_STYLE.entry, textStyle)}
          onClick = { () => this.props.addGroup(group)}
          key={group.id}>
        {group.name}
      </div>);
    }
    return (
      <div className={css(SuggestionBox.DYNAMIC_STYLE.box)}>
        {selectedGroups}
      </div>
    );
 }

  private static STYLE = {
    boxSmall: {
      maxHeight: '136px',
      overflow: 'auto' as 'auto',
      backroundColor: '#FFFFFF',
      boxShadow: '2px 0px 6px #C8C8C8',
      width: '100%'
    },
    boxMedium: {
      maxHeight: '136px',
      overflow: 'auto' as 'auto',
      backroundColor: '#FFFFFF',
      width: '100%'
    },
    boxLarge: {
      maxHeight: '136px',
      overflow: 'auto' as 'auto',
      backroundColor: '#FFFFFF',
      width: '100%'
    }
    //SCROLL BAR STYLES
  };

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
      ':hover': {
        color: '#FFFFFF',
        backgroundColor: '#684BC7'
      },
      ':active': {
        color: '#FFFFFF',
        backgroundColor: '#684BC7'
      }
    },
    textSmall: {
      font: '400 16px Roboto',
      color: '#000000'
    },
    textLarge: {
      font: '400 14px Roboto',
      color: '#000000'
    },
    box: {
      zIndex: 1,
      maxHeight: '136px',
      overflow: 'auto' as 'auto',
      backgroundColor: '#FFFFFF',
      boxShadow: '0px 2px 6px #C8C8C8',
      width: '100%',
      position: 'absolute' as 'absolute'
    }
 });

}


