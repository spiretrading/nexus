import {css, StyleSheet} from 'aphrodite';
import * as React from 'react';
import {BurgerButton, HBoxLayout, Padding, VBoxLayout} from '../..'

export interface Properties {}

export interface State {}

/** Displays the main dashboard. */
export class DashboardPage extends React.Component<Properties, State> {
  constructor(properties: Properties) {
    super(properties);
  }

  public render(): JSX.Element {
    return(
    	<VBoxLayout width='100%' height='100%'>
    		<VBoxLayout height='60px'>
    			<Padding size='15px'/>
    			<HBoxLayout width='136px'>
    				<Padding size='18px'/>
    				<BurgerButton width={50} height={50} color='#684BC7'
    				highlightColor='#FFFFFF'/>
    			</HBoxLayout>
    			<Padding size='15px'/>
    		</VBoxLayout>
    	</VBoxLayout>);
  }
}
