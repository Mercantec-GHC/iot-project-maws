using DomainModels;
using API.Data;
using Microsoft.AspNetCore.Authentication.JwtBearer;
using Microsoft.EntityFrameworkCore;
using Microsoft.IdentityModel.Tokens;
using System.Text;

var builder = WebApplication.CreateBuilder(args);

builder.Services.AddControllers();
builder.Services.AddEndpointsApiExplorer();
builder.Services.AddSwaggerGen();

// Connection to database.
builder.Services.AddDbContext<AppDBContext>(options => options.UseNpgsql(builder.Configuration.GetConnectionString("DefaultConnection")));

// Configure JWT authentication with HTTP support for Arduino
builder.Services.AddAuthentication(JwtBearerDefaults.AuthenticationScheme)
    .AddJwtBearer(options =>
    {
        options.TokenValidationParameters = new TokenValidationParameters
        {
            ValidateIssuer = true,
            ValidateAudience = true,
            ValidateLifetime = true,
            ValidateIssuerSigningKey = true,
            ValidIssuer = builder.Configuration["Jwt:Issuer"],
            ValidAudience = builder.Configuration["Jwt:Audience"],
            IssuerSigningKey = new SymmetricSecurityKey(Encoding.UTF8.GetBytes(builder.Configuration["Jwt:Key"]!))
        };

        // Allow HTTP requests for Arduino compatibility
        options.RequireHttpsMetadata = false;
        
        // Allow token from query string for Arduino if needed
        options.Events = new JwtBearerEvents
        {
            OnMessageReceived = context =>
            {
                var accessToken = context.Request.Query["access_token"];
                if (!string.IsNullOrEmpty(accessToken))
                {
                    context.Token = accessToken;
                }
                return Task.CompletedTask;
            }
        };
    });

// Add CORS configuration for Arduino and Blazor
builder.Services.AddCors(options =>
{
    options.AddPolicy("AllowArduinoAndBlazor", corsBuilder =>
    {
        corsBuilder.WithOrigins(
                "http://localhost:5143",    // Blazor HTTP
                "https://localhost:7091",   // Blazor HTTPS
                "http://localhost:7646",    // Blazor IIS Express
                "https://localhost:44349"   // Blazor IIS Express HTTPS
            )
            .AllowAnyMethod()
            .AllowAnyHeader()
            .AllowCredentials();
            
        // Also allow any origin for Arduino (you might want to restrict this in production)
        corsBuilder.SetIsOriginAllowed(origin => true)
            .AllowAnyMethod()
            .AllowAnyHeader();
    });
});

var app = builder.Build();

// Configure the HTTP request pipeline.
if (app.Environment.IsDevelopment())
{
    app.UseSwagger();
    app.UseSwaggerUI();
}

// DO NOT redirect HTTP to HTTPS for Arduino compatibility
// Remove or comment out: app.UseHttpsRedirection();

app.UseCors("AllowArduinoAndBlazor");

app.UseAuthentication();
app.UseAuthorization();

app.MapControllers();

app.Run();