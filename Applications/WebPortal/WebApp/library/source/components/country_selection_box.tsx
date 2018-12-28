import * as React from 'react';
import * as Nexus from 'nexus';

interface Properties {

  /** The set of available countries to select. */
  countryDatabase: Nexus.CountryDatabase;

  /** The currently selected country. */
  value: Nexus.CountryCode;

  /** Whether the selection box is read only. */
  readonly: boolean;

  /** The event handler called when the selection changes. */
  onChange?: (currency: Nexus.CountryCode) => void;
}

/** Displays a selection box for countries. */
export class CountrySelectionBox extends React.Component<Properties> {}
