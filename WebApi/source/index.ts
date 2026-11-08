import { getSide as getPositionSide, Position } from './accounting/position';
import { Quantity } from './definitions/quantity';
import { getSide as getQuantitySide, Side } from './definitions/side';

export * from './accounting';
export * from './administration_service';
export * from './compliance_service';
export * from './definitions';
export * from './definitions_service';
export * from './market_data_service';
export * from './queries';
export * from './risk_service';
export * from './service_clients';

/** Returns the side of a quantity. */
export function getSide(quantity: Quantity): Side;

/** Returns the side of a position. */
export function getSide(position: Position): Side;
export function getSide(value: Quantity | Position): Side {
  if(value instanceof Quantity) {
    return getQuantitySide(value);
  }
  return getPositionSide(value);
}
