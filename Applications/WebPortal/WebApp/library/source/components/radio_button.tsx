import { css, StyleSheet } from 'aphrodite/no-important';
import * as React from 'react';

interface Properties extends
    Omit<React.InputHTMLAttributes<HTMLInputElement>, 'type' | 'children'> {

  /** The input ID. Generated when omitted. */
  id?: string;

  /** The text associated with the radio input. */
  label: string;
}

/** A labeled native radio input. Generates an ID when none is supplied. */
export class RadioButton extends React.Component<Properties> {
  constructor(props: Properties) {
    super(props);
    this.identifier = `radio-button-${RadioButton.nextIdentifier++}`;
  }

  public render(): JSX.Element {
    const {label, id, className, ...rest} = this.props;
    const identifier = id ?? this.identifier;
    return <div className={css(STYLES.container)}>
      <input {...rest} type='radio' id={identifier}
        className={[css(STYLES.input), className].join(' ')}/>
      <Label id={identifier}>{label}</Label>
    </div>;
  }

  private static nextIdentifier = 0;
  private identifier: string;
}

interface LabelProperties {
  id: string;
  children: React.ReactNode;
}

function Label(props: LabelProperties): JSX.Element {
  return <label htmlFor={props.id} className={css(STYLES.label)}>
    {props.children}
  </label>;
}

const STYLES = StyleSheet.create({
  container: {display: 'inline-flex', alignItems: 'center'},
  input: {
    appearance: 'none',
    boxSizing: 'border-box',
    flexShrink: 0,
    padding: 0,
    backgroundColor: '#FFFFFF',
    border: '2px solid #C8C8C8',
    transition: 'border 60ms ease-out',
    cursor: 'pointer',
    ':checked': {border: '4px solid #684BC7'},
    ':disabled': {cursor: 'not-allowed', opacity: 0.4},
    ':disabled + label': {color: '#C8C8C8', cursor: 'not-allowed'},
    '@media (width < 768px)': {
      width: '20px', height: '20px', margin: 0, borderRadius: '10px'
    },
    '@media (min-width: 768px)': {
      width: '16px', height: '16px', margin: '2px', borderRadius: '8px'
    }
  },
  label: {
    display: 'flex',
    alignItems: 'center',
    minHeight: '20px',
    padding: '0 8px',
    fontSize: '0.875rem',
    color: '#333333',
    cursor: 'pointer'
  }
});
