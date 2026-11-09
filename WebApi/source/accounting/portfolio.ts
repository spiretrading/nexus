import { Money } from '../definitions/money';
import { Quantity } from '../definitions/quantity';
import { Inventory } from './inventory';
import { Valuation } from './valuation';

/**
 * Returns an inventory's realized profit and loss after fees.
 * @param inventory - The inventory to value.
 */
export function getRealizedProfitAndLoss(inventory: Inventory): Money {
  return inventory.grossProfitAndLoss.subtract(inventory.fees);
}

/**
 * Returns an inventory's unrealized profit and loss.
 * @param inventory - The inventory to value.
 * @param valuation - The bid and ask values to use.
 * @return Unrealized profit and loss, or null if a required price is absent.
 */
export function getUnrealizedProfitAndLoss(inventory: Inventory,
    valuation: Valuation): Money {
  const direction = inventory.position.quantity.compare(Quantity.ZERO);
  if(direction === 0) {
    return Money.ZERO;
  }
  let price: Money;
  if(direction > 0) {
    price = valuation.bidValue;
  } else {
    price = valuation.askValue;
  }
  if(!price) {
    return null;
  }
  return price.multiply(inventory.position.quantity.toJson() /
    Quantity.MULTIPLIER).subtract(inventory.position.costBasis);
}

/**
 * Returns an inventory's total realized and unrealized profit and loss.
 * @param inventory - The inventory to value.
 * @param valuation - The bid and ask values to use.
 * @return The total profit and loss, or null if a required price is absent.
 */
export function getTotalProfitAndLoss(inventory: Inventory,
    valuation: Valuation): Money {
  const unrealized = getUnrealizedProfitAndLoss(inventory, valuation);
  if(!unrealized) {
    return null;
  }
  return getRealizedProfitAndLoss(inventory).add(unrealized);
}
