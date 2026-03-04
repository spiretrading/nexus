import { css, StyleSheet } from 'aphrodite';
import * as React from 'react';

interface Properties {

  /** The status of the compliance rule. */
  status: ComplianceRuleStatusTag.Status;
}

/** Displays a colored tag indicating a compliance rule's status. */
export class ComplianceRuleStatusTag extends React.Component<Properties> {
  public render(): JSX.Element {
    const {label, color, backgroundColor, borderColor} =
      getStatusStyle(this.props.status);
    const dynamicStyle = StyleSheet.create({
      tag: {color, backgroundColor, borderColor}
    });
    return (
      <span className={css(STYLES.tag, dynamicStyle.tag)}>
        {label}
      </span>);
  }
}

export namespace ComplianceRuleStatusTag {
  export enum Status {
    ACTIVE,
    PASSIVE,
    DISABLED,
    DELETE,
    NONE
  }
}

function getStatusStyle(status: ComplianceRuleStatusTag.Status):
    {label: string, color: string, backgroundColor: string,
      borderColor: string} {
  switch(status) {
    case ComplianceRuleStatusTag.Status.ACTIVE:
      return {
        label: 'Active',
        color: '#0C7032',
        backgroundColor: '#EBFFF2',
        borderColor: 'transparent'
      };
    case ComplianceRuleStatusTag.Status.PASSIVE:
      return {
        label: 'Passive',
        color: '#1A3394',
        backgroundColor: '#F2F2FF',
        borderColor: 'transparent'
      };
    case ComplianceRuleStatusTag.Status.DISABLED:
      return {
        label: 'Disabled',
        color: '#7D7E90',
        backgroundColor: '#F8F8F8',
        borderColor: 'transparent'
      };
    case ComplianceRuleStatusTag.Status.DELETE:
      return {
        label: 'Delete',
        color: '#941B1B',
        backgroundColor: '#FFF1F1',
        borderColor: 'transparent'
      };
    case ComplianceRuleStatusTag.Status.NONE:
      return {
        label: 'None',
        color: '#7D7E90',
        backgroundColor: '#FFFFFF',
        borderColor: '#E6E6E6'
      };
  }
}

const STYLES = StyleSheet.create({
  tag: {
    display: 'inline-block',
    padding: '3px',
    border: '1px solid transparent',
    borderRadius: '4px',
    fontSize: '0.75rem',
    fontWeight: 500,
    fontFamily: 'Roboto',
    textAlign: 'center',
    minWidth: '60px'
  }
});
