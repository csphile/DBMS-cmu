with past_year(year, month) as(
    select date_year,
        date_month
    from release_info
    where (
            date_year == 2020
            and date_month <= 7
        )
        or (
            date_year == 2019
            and date_month >= 7
        )
)
select cast(year as varchar) || '.' || (iif (month < 10, '0', '')) || cast(month as varchar) as date,
    round(
        count(*) * 100.0 / (
            select count(*)
            from past_year
        ),
        2
    )
from past_year
group by date
order by date;