import React, { useState } from 'react';
import {
  TextField,
  InputAdornment,
  IconButton,
  Box,
  Chip,
  Typography,
  Paper,
  List,
  ListItem,
  ListItemText,
  ListItemIcon,
  Divider,
} from '@mui/material';
import {
  Search as SearchIcon,
  Clear as ClearIcon,
  FilterList as FilterIcon,
  History as HistoryIcon,
  TrendingUp as TrendingIcon,
} from '@mui/icons-material';

interface SearchSuggestion {
  id: string;
  text: string;
  type: 'recent' | 'trending' | 'suggestion';
  category?: string;
}

interface SearchBarProps {
  placeholder?: string;
  onSearch: (query: string) => void;
  suggestions?: SearchSuggestion[];
  showFilters?: boolean;
  onFilterClick?: () => void;
  recentSearches?: string[];
  onRecentSearchClick?: (query: string) => void;
  loading?: boolean;
}

const SearchBar: React.FC<SearchBarProps> = ({
  placeholder = 'Search...',
  onSearch,
  suggestions = [],
  showFilters = false,
  onFilterClick,
  recentSearches = [],
  onRecentSearchClick,
  loading = false,
}) => {
  const [query, setQuery] = useState('');
  const [showSuggestions, setShowSuggestions] = useState(false);
  const [focused, setFocused] = useState(false);

  const handleSearch = (searchQuery: string) => {
    if (searchQuery.trim()) {
      onSearch(searchQuery.trim());
      setShowSuggestions(false);
    }
  };

  const handleKeyPress = (event: React.KeyboardEvent) => {
    if (event.key === 'Enter') {
      handleSearch(query);
    }
  };

  const handleClear = () => {
    setQuery('');
    setShowSuggestions(false);
  };

  const handleSuggestionClick = (suggestion: SearchSuggestion) => {
    setQuery(suggestion.text);
    handleSearch(suggestion.text);
  };

  const handleRecentSearchClick = (recentQuery: string) => {
    setQuery(recentQuery);
    handleSearch(recentQuery);
  };

  const filteredSuggestions = suggestions.filter(suggestion =>
    suggestion.text.toLowerCase().includes(query.toLowerCase())
  );

  const recentSuggestions = recentSearches
    .filter(recent => recent.toLowerCase().includes(query.toLowerCase()))
    .slice(0, 5)
    .map(recent => ({
      id: `recent-${recent}`,
      text: recent,
      type: 'recent' as const,
    }));

  const allSuggestions = [
    ...recentSuggestions,
    ...filteredSuggestions,
  ].slice(0, 10);

  return (
    <Box position="relative" width="100%">
      <TextField
        fullWidth
        placeholder={placeholder}
        value={query}
        onChange={(e) => setQuery(e.target.value)}
        onKeyPress={handleKeyPress}
        onFocus={() => {
          setFocused(true);
          setShowSuggestions(true);
        }}
        onBlur={() => {
          setFocused(false);
          // Delay hiding suggestions to allow clicks
          setTimeout(() => setShowSuggestions(false), 200);
        }}
        InputProps={{
          startAdornment: (
            <InputAdornment position="start">
              <SearchIcon color="action" />
            </InputAdornment>
          ),
          endAdornment: (
            <Box display="flex" alignItems="center">
              {query && (
                <IconButton
                  size="small"
                  onClick={handleClear}
                  edge="end"
                >
                  <ClearIcon />
                </IconButton>
              )}
              {showFilters && onFilterClick && (
                <IconButton
                  size="small"
                  onClick={onFilterClick}
                  edge="end"
                >
                  <FilterIcon />
                </IconButton>
              )}
            </Box>
          ),
        }}
        sx={{
          '& .MuiOutlinedInput-root': {
            borderRadius: 2,
          },
        }}
      />

      {/* Search Suggestions */}
      {showSuggestions && (allSuggestions.length > 0 || recentSearches.length > 0) && (
        <Paper
          elevation={3}
          sx={{
            position: 'absolute',
            top: '100%',
            left: 0,
            right: 0,
            zIndex: 1000,
            mt: 1,
            maxHeight: 400,
            overflow: 'auto',
          }}
        >
          <List dense>
            {recentSearches.length > 0 && query && (
              <>
                <ListItem>
                  <ListItemIcon>
                    <HistoryIcon color="action" />
                  </ListItemIcon>
                  <ListItemText
                    primary="Recent Searches"
                    primaryTypographyProps={{
                      variant: 'subtitle2',
                      color: 'text.secondary',
                    }}
                  />
                </ListItem>
                <Divider />
              </>
            )}

            {allSuggestions.map((suggestion, index) => (
              <ListItem
                key={suggestion.id}
                button
                onClick={() => handleSuggestionClick(suggestion)}
                sx={{
                  '&:hover': {
                    backgroundColor: 'action.hover',
                  },
                }}
              >
                <ListItemIcon>
                  {suggestion.type === 'recent' && <HistoryIcon color="action" />}
                  {suggestion.type === 'trending' && <TrendingIcon color="primary" />}
                  {suggestion.type === 'suggestion' && <SearchIcon color="action" />}
                </ListItemIcon>
                <ListItemText
                  primary={suggestion.text}
                  secondary={suggestion.category}
                />
                {suggestion.type === 'trending' && (
                  <Chip
                    label="Trending"
                    size="small"
                    color="primary"
                    variant="outlined"
                  />
                )}
              </ListItem>
            ))}

            {allSuggestions.length === 0 && query && (
              <ListItem>
                <ListItemText
                  primary="No suggestions found"
                  primaryTypographyProps={{
                    variant: 'body2',
                    color: 'text.secondary',
                  }}
                />
              </ListItem>
            )}
          </List>
        </Paper>
      )}
    </Box>
  );
};

export default SearchBar;
