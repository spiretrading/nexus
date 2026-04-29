import * as React from 'react';
import { GroupProfitAndLossPage } from './group_profit_and_loss_page';

interface Properties {

  /** The username for the account. */
  username: string;

  /** The account currency symbol. */
  symbol: string;

  /** The total profit and loss in account currency. */
  totalPnl: string;

  /** The list of currencies with activity. */
  currencies: GroupProfitAndLossPage.CurrencyEntry[];
}

/** Displays an expandable account entry within the group P&L report. */
export function AccountProfitAndLossItem(props: Properties): JSX.Element {
  return <div>{props.username}</div>;
}
