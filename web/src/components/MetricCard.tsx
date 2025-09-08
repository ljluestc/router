import React from 'react';
import {
  Card,
  CardContent,
  Typography,
  Box,
  LinearProgress,
  Chip,
  IconButton,
  Tooltip,
} from '@mui/material';
import {
  TrendingUp as TrendingUpIcon,
  TrendingDown as TrendingDownIcon,
  TrendingFlat as TrendingFlatIcon,
  Info as InfoIcon,
  Refresh as RefreshIcon,
} from '@mui/icons-material';

export type TrendDirection = 'up' | 'down' | 'flat';

interface MetricCardProps {
  title: string;
  value: string | number;
  subtitle?: string;
  trend?: {
    direction: TrendDirection;
    value: number;
    label?: string;
  };
  progress?: {
    value: number;
    max?: number;
    color?: 'primary' | 'secondary' | 'success' | 'warning' | 'error';
  };
  status?: {
    label: string;
    color: 'success' | 'error' | 'warning' | 'info' | 'default';
  };
  icon?: React.ReactNode;
  tooltip?: string;
  onRefresh?: () => void;
  loading?: boolean;
  size?: 'small' | 'medium' | 'large';
}

const MetricCard: React.FC<MetricCardProps> = ({
  title,
  value,
  subtitle,
  trend,
  progress,
  status,
  icon,
  tooltip,
  onRefresh,
  loading = false,
  size = 'medium',
}) => {
  const getTrendIcon = (direction: TrendDirection) => {
    switch (direction) {
      case 'up':
        return <TrendingUpIcon color="success" />;
      case 'down':
        return <TrendingDownIcon color="error" />;
      case 'flat':
        return <TrendingFlatIcon color="action" />;
      default:
        return <TrendingFlatIcon color="action" />;
    }
  };

  const getTrendColor = (direction: TrendDirection) => {
    switch (direction) {
      case 'up':
        return 'success.main';
      case 'down':
        return 'error.main';
      case 'flat':
        return 'text.secondary';
      default:
        return 'text.secondary';
    }
  };

  const getSizeProps = () => {
    switch (size) {
      case 'small':
        return {
          titleVariant: 'body2' as const,
          valueVariant: 'h6' as const,
          subtitleVariant: 'caption' as const,
          iconSize: 20,
          padding: 2,
        };
      case 'large':
        return {
          titleVariant: 'h6' as const,
          valueVariant: 'h3' as const,
          subtitleVariant: 'body2' as const,
          iconSize: 32,
          padding: 3,
        };
      default: // medium
        return {
          titleVariant: 'body1' as const,
          valueVariant: 'h5' as const,
          subtitleVariant: 'body2' as const,
          iconSize: 24,
          padding: 2.5,
        };
    }
  };

  const sizeProps = getSizeProps();

  return (
    <Card
      sx={{
        height: '100%',
        display: 'flex',
        flexDirection: 'column',
        position: 'relative',
        '&:hover': {
          boxShadow: 4,
        },
      }}
    >
      <CardContent sx={{ p: sizeProps.padding, flexGrow: 1 }}>
        {/* Header */}
        <Box display="flex" justifyContent="space-between" alignItems="flex-start" mb={2}>
          <Box display="flex" alignItems="center" gap={1}>
            {icon && (
              <Box
                sx={{
                  color: 'primary.main',
                  display: 'flex',
                  alignItems: 'center',
                }}
              >
                {React.cloneElement(icon as React.ReactElement, {
                  sx: { fontSize: sizeProps.iconSize },
                })}
              </Box>
            )}
            <Typography
              variant={sizeProps.titleVariant}
              color="text.secondary"
              component="h3"
            >
              {title}
            </Typography>
            {tooltip && (
              <Tooltip title={tooltip} arrow>
                <IconButton size="small" sx={{ p: 0.5 }}>
                  <InfoIcon fontSize="small" />
                </IconButton>
              </Tooltip>
            )}
          </Box>
          <Box display="flex" alignItems="center" gap={1}>
            {status && (
              <Chip
                label={status.label}
                color={status.color}
                size="small"
                variant="outlined"
              />
            )}
            {onRefresh && (
              <IconButton
                size="small"
                onClick={onRefresh}
                disabled={loading}
                sx={{ p: 0.5 }}
              >
                <RefreshIcon fontSize="small" />
              </IconButton>
            )}
          </Box>
        </Box>

        {/* Value */}
        <Typography
          variant={sizeProps.valueVariant}
          component="div"
          sx={{
            fontWeight: 'bold',
            color: 'text.primary',
            mb: 1,
          }}
        >
          {loading ? '...' : value}
        </Typography>

        {/* Subtitle */}
        {subtitle && (
          <Typography
            variant={sizeProps.subtitleVariant}
            color="text.secondary"
            sx={{ mb: 2 }}
          >
            {subtitle}
          </Typography>
        )}

        {/* Trend */}
        {trend && (
          <Box display="flex" alignItems="center" gap={1} mb={2}>
            {getTrendIcon(trend.direction)}
            <Typography
              variant="body2"
              sx={{
                color: getTrendColor(trend.direction),
                fontWeight: 'medium',
              }}
            >
              {trend.value > 0 ? '+' : ''}{trend.value}%
              {trend.label && ` ${trend.label}`}
            </Typography>
          </Box>
        )}

        {/* Progress */}
        {progress && (
          <Box>
            <Box display="flex" justifyContent="space-between" alignItems="center" mb={1}>
              <Typography variant="body2" color="text.secondary">
                Progress
              </Typography>
              <Typography variant="body2" color="text.secondary">
                {progress.value}{progress.max ? ` / ${progress.max}` : '%'}
              </Typography>
            </Box>
            <LinearProgress
              variant="determinate"
              value={progress.max ? (progress.value / progress.max) * 100 : progress.value}
              color={progress.color || 'primary'}
              sx={{
                height: 6,
                borderRadius: 3,
              }}
            />
          </Box>
        )}
      </CardContent>
    </Card>
  );
};

export default MetricCard;
