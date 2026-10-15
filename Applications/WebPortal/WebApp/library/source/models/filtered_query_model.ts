import { QueryModel } from './query_model';

/** Filters another model's suggestions while preserving query parsing. */
export class FilteredQueryModel<T> extends QueryModel<T> {

  /** Constructs a filtered view of a query model.
   * @param model - The model that resolves queries.
   * @param filter - Returns whether a suggestion should be included.
   */
  constructor(model: QueryModel<T>, filter: (value: T) => boolean) {
    super();
    this.model = model;
    this.filter = filter;
  }

  public parse(query: string): Promise<T> {
    return this.model.parse(query);
  }

  public async submit(query: string): Promise<readonly T[]> {
    return (await this.model.submit(query)).filter(this.filter);
  }

  private model: QueryModel<T>;
  private filter: (value: T) => boolean;
}
