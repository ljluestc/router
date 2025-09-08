import React, { useState, useEffect } from 'react';
import {
  Typography,
  Tooltip,
} from '@mui/material';

interface TimeAgoProps {
  date: Date | string | number;
  variant?: 'body1' | 'body2' | 'caption' | 'subtitle1' | 'subtitle2';
  color?: 'textPrimary' | 'textSecondary' | 'primary' | 'secondary' | 'error' | 'warning' | 'info' | 'success';
  showTooltip?: boolean;
  tooltipFormat?: 'full' | 'relative' | 'custom';
  customTooltipFormat?: (date: Date) => string;
  updateInterval?: number;
  precision?: 'second' | 'minute' | 'hour' | 'day';
}

const TimeAgo: React.FC<TimeAgoProps> = ({
  date,
  variant = 'body2',
  color = 'textSecondary',
  showTooltip = true,
  tooltipFormat = 'full',
  customTooltipFormat,
  updateInterval = 1000,
  precision = 'minute',
}) => {
  const [timeAgo, setTimeAgo] = useState('');

  const formatDate = (date: Date | string | number): Date => {
    if (date instanceof Date) {
      return date;
    }
    if (typeof date === 'string' || typeof date === 'number') {
      return new Date(date);
    }
    return new Date();
  };

  const formatTimeAgo = (date: Date): string => {
    const now = new Date();
    const diffInSeconds = Math.floor((now.getTime() - date.getTime()) / 1000);

    if (diffInSeconds < 60) {
      return 'just now';
    }

    const diffInMinutes = Math.floor(diffInSeconds / 60);
    if (diffInMinutes < 60) {
      return `${diffInMinutes} minute${diffInMinutes === 1 ? '' : 's'} ago`;
    }

    const diffInHours = Math.floor(diffInMinutes / 60);
    if (diffInHours < 24) {
      return `${diffInHours} hour${diffInHours === 1 ? '' : 's'} ago`;
    }

    const diffInDays = Math.floor(diffInHours / 24);
    if (diffInDays < 7) {
      return `${diffInDays} day${diffInDays === 1 ? '' : 's'} ago`;
    }

    const diffInWeeks = Math.floor(diffInDays / 7);
    if (diffInWeeks < 4) {
      return `${diffInWeeks} week${diffInWeeks === 1 ? '' : 's'} ago`;
    }

    const diffInMonths = Math.floor(diffInDays / 30);
    if (diffInMonths < 12) {
      return `${diffInMonths} month${diffInMonths === 1 ? '' : 's'} ago`;
    }

    const diffInYears = Math.floor(diffInDays / 365);
    return `${diffInYears} year${diffInYears === 1 ? '' : 's'} ago`;
  };

  const formatTooltip = (date: Date): string => {
    if (customTooltipFormat) {
      return customTooltipFormat(date);
    }

    switch (tooltipFormat) {
      case 'full':
        return date.toLocaleString();
      case 'relative':
        return formatTimeAgo(date);
      default:
        return date.toLocaleString();
    }
  };

  const updateTimeAgo = () => {
    const formattedDate = formatDate(date);
    setTimeAgo(formatTimeAgo(formattedDate));
  };

  useEffect(() => {
    updateTimeAgo();

    const interval = setInterval(updateTimeAgo, updateInterval);

    return () => clearInterval(interval);
  }, [date, updateInterval]);

  const formattedDate = formatDate(date);
  const tooltipText = formatTooltip(formattedDate);

  const content = (
    <Typography
      variant={variant}
      color={color}
      component="span"
    >
      {timeAgo}
    </Typography>
  );

  if (showTooltip) {
    return (
      <Tooltip title={tooltipText} arrow>
        {content}
      </Tooltip>
    );
  }

  return content;
};

export default TimeAgo;
