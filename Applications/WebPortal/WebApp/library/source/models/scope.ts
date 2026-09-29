import * as Beam from 'beam';
import * as Nexus from 'nexus';

/** Displays a scope using country codes, venue names, and qualified tickers. */
export function getScopeLabel(scope: Nexus.Scope): string {
  if(scope.isGlobal) {
    return '*';
  }
  const countries = Array.from(scope.countries, country =>
    Nexus.countryDatabase.fromCode(country).twoLetterCode).sort();
  const venues = Array.from(scope.venues, venue =>
    Nexus.venueDatabase.fromVenue(venue).displayName || venue.toString()).sort();
  const tickers = Array.from(
    scope.tickers, ticker => ticker.toString()).sort();
  return [...countries, ...venues, ...tickers].join(', ');
}

/** Compares scope membership, ignoring the optional display name. */
export function isScopeEqual(first: Nexus.Scope, second: Nexus.Scope): boolean {
  return first.isGlobal === second.isGlobal &&
    equalSets(first.countries, second.countries) &&
    equalSets(first.venues, second.venues) &&
    equalSets(first.tickers, second.tickers);
}

/** Splits a scope into individual countries, venues, and tickers. */
export function splitScope(scope: Nexus.Scope): Nexus.Scope[] {
  if(scope.isGlobal) {
    return [scope.clone()];
  }
  const countries = Array.from(
    scope.countries, value => new Nexus.Scope(value));
  const venues = Array.from(scope.venues, value => new Nexus.Scope(value));
  const tickers = Array.from(scope.tickers, value => new Nexus.Scope(value));
  const compare = (first: Nexus.Scope, second: Nexus.Scope) =>
    getScopeLabel(first).localeCompare(getScopeLabel(second));
  return [...countries.sort(compare), ...venues.sort(compare),
    ...tickers.sort(compare)];
}

/** Combines scope entries into a named scope. Global includes everything. */
export function combineScopes(scopes: readonly Nexus.Scope[], name: string):
    Nexus.Scope {
  const scope = new Nexus.Scope(name);
  for(const entry of scopes) {
    scope.add(entry);
  }
  return scope;
}

function equalSets<T>(first: Beam.Set<T>, second: Beam.Set<T>): boolean {
  return first.size === second.size &&
    Array.from(first).every(value => second.has(value));
}
