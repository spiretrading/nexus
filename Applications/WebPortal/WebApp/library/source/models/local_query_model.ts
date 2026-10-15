import { QueryModel } from './query_model';

/** Resolves exact names and case-insensitive prefixes in a local collection. */
export class LocalQueryModel<T> extends QueryModel<T> {

  /** Constructs an empty model.
   * @param getLabel - Returns the default name used to query a value.
   * @param isEqual - Compares values when combining matches from aliases.
   *                 Defaults to Object.is.
   */
  constructor(getLabel: (value: T) => string,
      isEqual?: (first: T, second: T) => boolean) {
    super();
    this.getLabel = getLabel;
    this.isEqual = isEqual ?? Object.is;
    this.values = new Map();
  }

  /** Adds a value using its default name, replacing any existing mapping. */
  public add(value: T): void;

  /** Associates a name or alias with a value, replacing any existing mapping.
   * @param name - The case-insensitive name used to query the value.
   * @param value - The value associated with the name.
   */
  public add(name: string, value: T): void;
  public add(name: string | T, value?: T): void {
    if(arguments.length === 1) {
      const item = name as T;
      this.add(this.getLabel(item), item);
    } else {
      this.values.set((name as string).toLocaleLowerCase(), value);
    }
  }

  public async parse(query: string): Promise<T> {
    return this.values.get(query.toLocaleLowerCase()) ?? null;
  }

  public async submit(query: string): Promise<readonly T[]> {
    const prefix = query.toLocaleLowerCase();
    const matches: T[] = [];
    for(const [name, value] of this.values) {
      if(name.startsWith(prefix) &&
          !matches.some(match => this.isEqual(match, value))) {
        matches.push(value);
      }
    }
    return matches;
  }

  private getLabel: (value: T) => string;
  private isEqual: (first: T, second: T) => boolean;
  private values: Map<string, T>;
}
