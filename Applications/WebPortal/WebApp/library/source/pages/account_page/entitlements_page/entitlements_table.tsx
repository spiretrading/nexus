import * as Beam from 'beam';
import * as Nexus from 'nexus';
import * as React from 'react';

interface Properties {

  /** Stores the entitlements to display. */
  entitlements: Nexus.EntitlementDatabase;

  /** The set of entitlements that are checked. */
  checked: Beam.Set<Beam.DirectoryEntry>;

  /** Indicates an entitlement has been clicked.
   * @param entitlement - The entitlement that was clicked.
   */
  onClick?: (entitlement: Beam.DirectoryEntry) => void;
}

/** Displays a list of entitlements in a table. */
export class EntitlementsTable extends React.Component<Properties> {
  public static readonly defaultProps = {
    onClick: () => {}
  }

  constructor(props: Properties) {
    super(props);
  }
}
