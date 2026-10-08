import * as React from 'react';
import { QueryModel } from '../models';
import { Input } from './input';
import { SuggestionsWindow } from './suggestions_window';

interface Properties<T> extends
    Omit<React.InputHTMLAttributes<HTMLInputElement>,
      'value' | 'defaultValue' | 'onChange'> {

  /** The committed value. Null represents an empty field. */
  value: T;

  /** Resolves typed values and supplies suggestions. */
  model: QueryModel<T>;

  /** Returns a value's display text. */
  getLabel: (value: T) => string;

  /** Compares non-null values. Defaults to Object.is. */
  isEqual?: (first: T, second: T) => boolean;

  /** The validation message for an unresolved value. */
  invalidMessage?: string;

  /** Resolves typed text when focus leaves the input. Defaults to true. */
  commitOnBlur?: boolean;

  /** Called when a value is selected or successfully parsed by the model. */
  onChange?: (value: T) => void;

  /** Called when resolving a typed value starts or finishes. */
  onResolvingChange?: (resolving: boolean) => void;

  /** Called when the input validation message changes. */
  onValidationError?: (message: string) => void;
}

interface State<T> {
  query: string;
  suggestions: readonly T[];
  highlighted: number;
  suggestionSelected: boolean;
  expanded: boolean;
  searching: boolean;
  resolving: boolean;
  error: string;
}

/** Edits one value using model suggestions or a directly entered value. */
export class ComboBox<T> extends React.Component<Properties<T>, State<T>> {
  constructor(props: Properties<T>) {
    super(props);
    this.state = {query: this.getText(props.value), suggestions: [],
      highlighted: 0, suggestionSelected: false, expanded: false,
      searching: false, resolving: false, error: ''};
    this.input = React.createRef<HTMLInputElement>();
    this.searchVersion = 0;
    this.resolveVersion = 0;
    this.timer = null;
    this.searchResult = Promise.resolve([]);
    this.completeSearch = null;
    this.identifier = `combo-${ComboBox.nextIdentifier++}`;
  }

  /** Moves keyboard focus to the input field. */
  public focus(): void {
    this.input.current?.focus();
  }

  /** Discards pending queries and restores the committed value. */
  public reset(): void {
    this.cancelSearch();
    ++this.resolveVersion;
    this.setState({query: this.getText(this.props.value), expanded: false,
      suggestions: [], searching: false, resolving: false, error: ''});
  }

  /** Reloads suggestions for the current query without changing the input. */
  public refresh(): void {
    this.cancelSearch();
    ++this.resolveVersion;
    this.setState({suggestions: [], highlighted: 0, suggestionSelected: false,
      searching: false, resolving: false, error: ''});
    if(this.state.expanded && !this.props.readOnly && !this.props.disabled) {
      this.search(this.state.query);
    }
  }

  public componentDidUpdate(previous: Properties<T>, state: State<T>): void {
    if(previous.value !== this.props.value ||
        previous.model !== this.props.model ||
        previous.readOnly !== this.props.readOnly ||
        previous.disabled !== this.props.disabled) {
      this.reset();
    }
    if(state.resolving !== this.state.resolving) {
      this.props.onResolvingChange?.(this.state.resolving);
    }
    const error = getValidationMessage(this.state);
    if(getValidationMessage(state) !== error) {
      this.props.onValidationError?.(error);
    }
    let message = error;
    if(!message && this.state.query !== '' && (this.state.resolving ||
        this.state.query !== this.getText(this.props.value))) {
      message = 'Finish entering or selecting a value.';
    }
    this.input.current.setCustomValidity(message);
  }

  public componentWillUnmount(): void {
    this.cancelSearch();
    ++this.resolveVersion;
  }

  public render(): JSX.Element {
    const {value, model, getLabel, isEqual, invalidMessage, onChange, onKeyDown,
      onBlur, commitOnBlur, onResolvingChange, onValidationError, ...rest} =
        this.props;
    const suggestions = this.state.suggestions;
    const expanded = this.state.expanded && !rest.readOnly && !rest.disabled;
    const error = getValidationMessage(this.state);
    const active = (() => {
      if(expanded && suggestions[this.state.highlighted] !== undefined) {
        return `${this.identifier}-${this.state.highlighted}`;
      }
      return undefined;
    })();
    return <>
      <Input {...rest} ref={this.input} type='text' role='combobox'
        autoComplete='off' aria-autocomplete='list'
        aria-expanded={expanded} aria-controls={this.identifier}
        aria-activedescendant={active}
        aria-busy={this.state.searching || this.state.resolving}
        aria-invalid={Boolean(error) || rest['aria-invalid']}
        data-pending={!error && (this.state.resolving ||
          this.state.query !== this.getText(this.props.value))}
        value={this.state.query} style={{width: '100%', ...rest.style}}
        onChange={this.onQuery} onKeyDown={this.onKeyDown}
        onBlur={this.onBlur}/>
      {expanded && <SuggestionsWindow anchor={this.input.current}
        id={this.identifier} items={suggestions}
        selected={this.state.highlighted} getLabel={getLabel}
        loading={this.state.searching || this.state.resolving}
        onHighlight={this.onHighlight} onSubmit={this.onSelect}/>}
    </>;
  }

  private getText(value: T): string {
    if(value == null) {
      return '';
    }
    return this.props.getLabel(value);
  }

  private equals(first: T, second: T): boolean {
    if(first == null || second == null) {
      return first == second;
    }
    return (this.props.isEqual ?? Object.is)(first, second);
  }

  private cancelSearch(): void {
    ++this.searchVersion;
    this.completeSearch?.([]);
    this.completeSearch = null;
    this.searchResult = Promise.resolve([]);
    if(this.timer !== null) {
      window.clearTimeout(this.timer);
      this.timer = null;
    }
  }

  private search(query: string): void {
    if(query === '') {
      return;
    }
    const version = this.searchVersion;
    const model = this.props.model;
    let complete: (values: readonly T[]) => void;
    this.searchResult = new Promise(resolve => {
      complete = resolve;
    });
    this.completeSearch = complete;
    this.setState({searching: true});
    this.timer = window.setTimeout(async () => {
      this.timer = null;
      try {
        const values = await model.submit(query);
        if(version !== this.searchVersion) {
          return;
        }
        const suggestions = values.filter((item, i) =>
          values.findIndex(other => this.equals(item, other)) === i);
        this.setState(state => ({suggestions, searching: false,
          highlighted: 0, error: (() => {
            if(suggestions.length !== 0) {
              return '';
            }
            return state.error;
          })()}));
        this.completeSearch = null;
        complete(suggestions);
      } catch {
        if(version === this.searchVersion) {
          this.setState({suggestions: [], searching: false});
          this.completeSearch = null;
          complete([]);
        }
      }
    }, 200);
  }

  private commit(value: T): void {
    this.cancelSearch();
    ++this.resolveVersion;
    this.setState({query: this.getText(value), suggestions: [], expanded: false,
      searching: false, resolving: false, error: ''});
    if(!this.equals(value, this.props.value)) {
      this.props.onChange?.(value);
    }
  }

  private async resolve(selectSuggestion: boolean): Promise<void> {
    if(this.state.resolving) {
      return;
    }
    const version = ++this.resolveVersion;
    const searchResult = this.searchResult;
    const highlighted = this.state.highlighted;
    this.setState({resolving: true, error: ''});
    try {
      const value = await this.props.model.parse(this.state.query);
      if(version !== this.resolveVersion) {
        return;
      }
      if(value !== null) {
        this.commit(value);
        return;
      }
      if(selectSuggestion) {
        const suggestions = await searchResult;
        if(version !== this.resolveVersion) {
          return;
        }
        const fallback = suggestions[highlighted] ?? null;
        if(fallback !== null) {
          this.commit(fallback);
          return;
        }
      }
      this.setState({resolving: false,
        error: this.props.invalidMessage ?? 'Unknown or ambiguous item.'});
    } catch {
      if(version === this.resolveVersion) {
        this.setState(
          {resolving: false, error: 'Unable to resolve this item. Try again.'});
      }
    }
  }

  private onQuery = (event: React.ChangeEvent<HTMLInputElement>) => {
    this.cancelSearch();
    ++this.resolveVersion;
    const query = event.target.value;
    this.setState({query, suggestions: [], highlighted: 0,
      suggestionSelected: false, expanded: query !== '', searching: false,
      resolving: false, error: ''});
    this.search(query);
  };

  private onKeyDown = (event: React.KeyboardEvent<HTMLInputElement>) => {
    this.props.onKeyDown?.(event);
    if(event.defaultPrevented || event.nativeEvent.isComposing ||
        this.props.readOnly || this.props.disabled) {
      return;
    }
    const suggestions = this.state.suggestions;
    if(event.key === 'ArrowDown' || event.key === 'ArrowUp') {
      if(!this.state.expanded || suggestions.length === 0) {
        return;
      }
      event.preventDefault();
      let highlighted = this.state.highlighted;
      if(event.key === 'ArrowDown') {
        highlighted = Math.min(highlighted + 1, suggestions.length - 1);
      } else {
        highlighted = Math.max(highlighted - 1, 0);
      }
      this.setState({highlighted, suggestionSelected: true});
    } else if(event.key === 'Enter') {
      event.preventDefault();
      let fallback: T = null;
      if(this.state.expanded) {
        fallback = suggestions[this.state.highlighted] ?? null;
      }
      if(this.state.suggestionSelected && fallback !== null) {
        this.commit(fallback);
      } else {
        this.resolve(this.state.expanded);
      }
    } else if(event.key === 'Escape' && (this.state.expanded ||
        this.state.query !== this.getText(this.props.value))) {
      event.preventDefault();
      event.stopPropagation();
      this.reset();
    }
  };

  private onBlur = (event: React.FocusEvent<HTMLInputElement>) => {
    if(!this.state.resolving) {
      this.cancelSearch();
    }
    this.setState({expanded: false, searching: false});
    if(this.props.commitOnBlur !== false && !this.props.readOnly &&
        !this.props.disabled &&
        this.state.query !== this.getText(this.props.value)) {
      this.resolve(false);
    }
    this.props.onBlur?.(event);
  };

  private onHighlight = (highlighted: number) => {
    this.setState({highlighted, suggestionSelected: true});
  };

  private onSelect = (value: T) => {
    this.commit(value);
  };

  private static nextIdentifier = 0;
  private input: React.RefObject<HTMLInputElement>;
  private searchVersion: number;
  private resolveVersion: number;
  private timer: number;
  private searchResult: Promise<readonly T[]>;
  private completeSearch: (values: readonly T[]) => void;
  private identifier: string;
}

function getValidationMessage<T>(state: State<T>): string {
  if(state.query === '') {
    return '';
  }
  return state.error;
}
