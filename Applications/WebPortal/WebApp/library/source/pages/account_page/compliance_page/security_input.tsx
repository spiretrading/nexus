import * as React from 'react';

interface Properties {
  isEditing?: boolean;
}

export class SecurityInput extends React.Component<Properties>{
  public render() {
    const visibility = (() => {
      if(this.props.isEditing) {
        return SecurityInput.STYLE.hidden;
      } else {
        return null;
      }
    })();
    return(
      <div>
        <div> After how many characters do you do the ...???? </div>
        <div style={visibility}>
          
        </div>
      </div>);
  }
  private static readonly STYLE = {
    hidden: {
      visibility: 'hidden' as 'hidden',
      display: 'none' as 'none'
    }
  };
}