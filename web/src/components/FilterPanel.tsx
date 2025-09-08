import React, { useState } from 'react';
import {
  Box,
  Typography,
  Paper,
  TextField,
  FormControl,
  InputLabel,
  Select,
  MenuItem,
  Checkbox,
  FormControlLabel,
  Slider,
  Button,
  Chip,
  Accordion,
  AccordionSummary,
  AccordionDetails,
  Divider,
  IconButton,
  Tooltip,
} from '@mui/material';
import {
  ExpandMore as ExpandMoreIcon,
  Clear as ClearIcon,
  FilterList as FilterIcon,
  Close as CloseIcon,
} from '@mui/icons-material';

export interface FilterOption {
  id: string;
  label: string;
  value: any;
  type: 'text' | 'select' | 'multiselect' | 'checkbox' | 'slider' | 'date' | 'daterange';
  options?: { value: any; label: string }[];
  min?: number;
  max?: number;
  step?: number;
  multiple?: boolean;
}

export interface FilterPanelProps {
  title?: string;
  filters: FilterOption[];
  values: Record<string, any>;
  onChange: (values: Record<string, any>) => void;
  onClear?: () => void;
  onApply?: () => void;
  collapsible?: boolean;
  defaultExpanded?: boolean;
  showClearButton?: boolean;
  showApplyButton?: boolean;
  loading?: boolean;
}

const FilterPanel: React.FC<FilterPanelProps> = ({
  title = 'Filters',
  filters,
  values,
  onChange,
  onClear,
  onApply,
  collapsible = false,
  defaultExpanded = true,
  showClearButton = true,
  showApplyButton = false,
  loading = false,
}) => {
  const [expanded, setExpanded] = useState(defaultExpanded);

  const handleFilterChange = (filterId: string, value: any) => {
    const newValues = { ...values, [filterId]: value };
    onChange(newValues);
  };

  const handleClear = () => {
    const clearedValues = filters.reduce((acc, filter) => {
      acc[filter.id] = filter.type === 'multiselect' ? [] : '';
      return acc;
    }, {} as Record<string, any>);
    onChange(clearedValues);
    onClear?.();
  };

  const handleApply = () => {
    onApply?.();
  };

  const getActiveFilterCount = () => {
    return filters.filter(filter => {
      const value = values[filter.id];
      if (filter.type === 'multiselect') {
        return Array.isArray(value) && value.length > 0;
      }
      return value !== undefined && value !== '' && value !== null;
    }).length;
  };

  const renderFilter = (filter: FilterOption) => {
    const value = values[filter.id] || (filter.type === 'multiselect' ? [] : '');

    switch (filter.type) {
      case 'text':
        return (
          <TextField
            fullWidth
            label={filter.label}
            value={value}
            onChange={(e) => handleFilterChange(filter.id, e.target.value)}
            size="small"
            disabled={loading}
          />
        );

      case 'select':
        return (
          <FormControl fullWidth size="small" disabled={loading}>
            <InputLabel>{filter.label}</InputLabel>
            <Select
              value={value}
              onChange={(e) => handleFilterChange(filter.id, e.target.value)}
              label={filter.label}
            >
              {filter.options?.map((option) => (
                <MenuItem key={option.value} value={option.value}>
                  {option.label}
                </MenuItem>
              ))}
            </Select>
          </FormControl>
        );

      case 'multiselect':
        return (
          <FormControl fullWidth size="small" disabled={loading}>
            <InputLabel>{filter.label}</InputLabel>
            <Select
              multiple
              value={value}
              onChange={(e) => handleFilterChange(filter.id, e.target.value)}
              label={filter.label}
              renderValue={(selected) => (
                <Box sx={{ display: 'flex', flexWrap: 'wrap', gap: 0.5 }}>
                  {selected.map((val: any) => {
                    const option = filter.options?.find(opt => opt.value === val);
                    return (
                      <Chip
                        key={val}
                        label={option?.label || val}
                        size="small"
                      />
                    );
                  })}
                </Box>
              )}
            >
              {filter.options?.map((option) => (
                <MenuItem key={option.value} value={option.value}>
                  <Checkbox checked={value.includes(option.value)} />
                  {option.label}
                </MenuItem>
              ))}
            </Select>
          </FormControl>
        );

      case 'checkbox':
        return (
          <FormControlLabel
            control={
              <Checkbox
                checked={value}
                onChange={(e) => handleFilterChange(filter.id, e.target.checked)}
                disabled={loading}
              />
            }
            label={filter.label}
          />
        );

      case 'slider':
        return (
          <Box>
            <Typography variant="body2" gutterBottom>
              {filter.label}: {value}
            </Typography>
            <Slider
              value={value}
              onChange={(_, newValue) => handleFilterChange(filter.id, newValue)}
              min={filter.min || 0}
              max={filter.max || 100}
              step={filter.step || 1}
              disabled={loading}
            />
          </Box>
        );

      case 'date':
        return (
          <TextField
            fullWidth
            label={filter.label}
            type="date"
            value={value}
            onChange={(e) => handleFilterChange(filter.id, e.target.value)}
            size="small"
            disabled={loading}
            InputLabelProps={{
              shrink: true,
            }}
          />
        );

      case 'daterange':
        return (
          <Box display="flex" gap={1}>
            <TextField
              fullWidth
              label="From"
              type="date"
              value={value?.from || ''}
              onChange={(e) => handleFilterChange(filter.id, { ...value, from: e.target.value })}
              size="small"
              disabled={loading}
              InputLabelProps={{
                shrink: true,
              }}
            />
            <TextField
              fullWidth
              label="To"
              type="date"
              value={value?.to || ''}
              onChange={(e) => handleFilterChange(filter.id, { ...value, to: e.target.value })}
              size="small"
              disabled={loading}
              InputLabelProps={{
                shrink: true,
              }}
            />
          </Box>
        );

      default:
        return null;
    }
  };

  const content = (
    <Box>
      {/* Header */}
      <Box display="flex" justifyContent="space-between" alignItems="center" mb={2}>
        <Box display="flex" alignItems="center" gap={1}>
          <FilterIcon color="action" />
          <Typography variant="h6">
            {title}
          </Typography>
          {getActiveFilterCount() > 0 && (
            <Chip
              label={getActiveFilterCount()}
              size="small"
              color="primary"
            />
          )}
        </Box>
        <Box display="flex" gap={1}>
          {showClearButton && (
            <Tooltip title="Clear all filters">
              <IconButton size="small" onClick={handleClear} disabled={loading}>
                <ClearIcon />
              </IconButton>
            </Tooltip>
          )}
          {collapsible && (
            <IconButton
              size="small"
              onClick={() => setExpanded(!expanded)}
            >
              <ExpandMoreIcon
                sx={{
                  transform: expanded ? 'rotate(180deg)' : 'rotate(0deg)',
                  transition: 'transform 0.2s',
                }}
              />
            </IconButton>
          )}
        </Box>
      </Box>

      {/* Filters */}
      {expanded && (
        <Box>
          <Box display="flex" flexDirection="column" gap={2}>
            {filters.map((filter) => (
              <Box key={filter.id}>
                {renderFilter(filter)}
              </Box>
            ))}
          </Box>

          {/* Actions */}
          {(showApplyButton || showClearButton) && (
            <Box mt={3}>
              <Divider sx={{ mb: 2 }} />
              <Box display="flex" gap={1} justifyContent="flex-end">
                {showClearButton && (
                  <Button
                    variant="outlined"
                    startIcon={<ClearIcon />}
                    onClick={handleClear}
                    disabled={loading}
                  >
                    Clear
                  </Button>
                )}
                {showApplyButton && (
                  <Button
                    variant="contained"
                    onClick={handleApply}
                    disabled={loading}
                  >
                    Apply
                  </Button>
                )}
              </Box>
            </Box>
          )}
        </Box>
      )}
    </Box>
  );

  if (collapsible) {
    return (
      <Accordion
        expanded={expanded}
        onChange={() => setExpanded(!expanded)}
        sx={{ mb: 2 }}
      >
        <AccordionSummary expandIcon={<ExpandMoreIcon />}>
          <Box display="flex" alignItems="center" gap={1}>
            <FilterIcon color="action" />
            <Typography variant="h6">
              {title}
            </Typography>
            {getActiveFilterCount() > 0 && (
              <Chip
                label={getActiveFilterCount()}
                size="small"
                color="primary"
              />
            )}
          </Box>
        </AccordionSummary>
        <AccordionDetails>
          <Box display="flex" flexDirection="column" gap={2}>
            {filters.map((filter) => (
              <Box key={filter.id}>
                {renderFilter(filter)}
              </Box>
            ))}
          </Box>
          {(showApplyButton || showClearButton) && (
            <Box mt={3}>
              <Divider sx={{ mb: 2 }} />
              <Box display="flex" gap={1} justifyContent="flex-end">
                {showClearButton && (
                  <Button
                    variant="outlined"
                    startIcon={<ClearIcon />}
                    onClick={handleClear}
                    disabled={loading}
                  >
                    Clear
                  </Button>
                )}
                {showApplyButton && (
                  <Button
                    variant="contained"
                    onClick={handleApply}
                    disabled={loading}
                  >
                    Apply
                  </Button>
                )}
              </Box>
            </Box>
          )}
        </AccordionDetails>
      </Accordion>
    );
  }

  return (
    <Paper elevation={1} sx={{ p: 2, mb: 2 }}>
      {content}
    </Paper>
  );
};

export default FilterPanel;
